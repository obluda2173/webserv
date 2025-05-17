#include "CgiHandler.h"
#include "handlerUtils.h"

std::string CgiHandler::_extractQuery(const std::string& uri) {
    const size_t pos = uri.find('?');
    return (pos != std::string::npos) ? uri.substr(pos + 1) : "";
}

std::string CgiHandler::_findInterpreter(std::map< std::string, std::string > cgiMap) {
    size_t dot = _path.rfind('.');
    if (dot == std::string::npos)
        return "";
    size_t end = _path.find_first_of("/?#", dot);
    std::string ext = _path.substr(dot + 1, end - dot - 1);
    std::map< std::string, std::string >::const_iterator it = cgiMap.find(ext);
    return it != cgiMap.end() ? it->second : "";
}

bool CgiHandler::_validateAndPrepareContext(const HttpRequest& request, const RouteConfig& config, HttpResponse& resp) {
    _path = normalizePath(config.root, request.uri);
    _query = _extractQuery(request.uri);
    _interpreter = _findInterpreter(config.cgi);
    if (_interpreter.empty()) {
        setErrorResponse(resp, 403, "Forbidden", config);
        return false;
    }
    return validateRequest(resp, request, config, _path, _pathStat);
}

void CgiHandler::_setCgiEnvironment(const HttpRequest& request) {
    _envStorage.push_back("GATEWAY_INTERFACE=CGI/1.1");
    _envStorage.push_back("SERVER_PROTOCOL=" + request.version);
    _envStorage.push_back("REQUEST_METHOD=" + request.method);
    _envStorage.push_back("SCRIPT_NAME=" + _path);
    _envStorage.push_back("QUERY_STRING=" + _query);
    if (request.headers.count("content-length")) {
        _envStorage.push_back("CONTENT_LENGTH=" + request.headers.at("content-length"));
    }
    if (request.headers.count("content-type")) {
        _envStorage.push_back("CONTENT_TYPE=" + request.headers.at("content-type"));
    }
    // not complete
}

void CgiHandler::_prepareExecParams(const HttpRequest& request, ExecParams& params) {
    params.argv.push_back(_interpreter.c_str());
    params.argv.push_back(_path.c_str());
    params.argv.push_back(NULL);

    _setCgiEnvironment(request);
    for (size_t i = 0; i < _envStorage.size(); i++) {
        params.env.push_back(_envStorage[i].c_str());
    }
    params.env.push_back(NULL);
}

void CgiHandler::_setupChildProcess(int pipefd[2]) {
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[1]);
}

void CgiHandler::_setupParentProcess(Connection* conn, int pipefd[2], pid_t pid, const RouteConfig& config) {
    close(pipefd[1]);
    fcntl(pipefd[0], F_SETFL, O_NONBLOCK);
    conn->cgiCtx.cgiPid = pid;
    conn->cgiCtx.cgiPipeFd = pipefd[0];
    conn->cgiCtx.cgiRouteConfig = config;
    conn->setState(Connection::HandlingCgi);
}

void CgiHandler::_parseCgiOutput(const std::string& cgiOutput, HttpResponse& resp) {
    (void)cgiOutput;
    (void)resp;
}

ProcessState CgiHandler::_checkProcess(CgiContext& ctx, int& status) {
    pid_t result = waitpid(ctx.cgiPid, &status, WNOHANG);
    if (result == -1) {
        return ProcessState::Error;
    }
    return result > 0 ? ProcessState::Exited : ProcessState::Running;
}

bool CgiHandler::_readPipeData(CgiContext& cgiCtx, bool drain) {
    if (cgiCtx.cgiPipeFd == -1) {
        return false;
    }

    do {
        char buffer[4096];
        const ssize_t count = read(cgiCtx.cgiPipeFd, buffer, sizeof(buffer));
        if (count > 0) {
            cgiCtx.cgiOutput.append(buffer, count);
        } else {
            if (count == 0 || (count == -1 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                close(cgiCtx.cgiPipeFd);
                cgiCtx.cgiPipeFd = -1;

                if (count == -1) {
                    return false;
                }
            }
            break;
        }
    } while (drain);
    return true;
}

void CgiHandler::_handleProcessExit(Connection* conn, CgiContext& ctx, int status) {
    if (ctx.cgiPipeFd != -1) {
        _readPipeData(ctx, true);
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        setErrorResponse(conn->_response, 502, "Bad Gateway", ctx.cgiRouteConfig);
    } else if (ctx.cgiOutput.empty()) {
        setErrorResponse(conn->_response, 500, "Internal Error", ctx.cgiRouteConfig);
    } else {
        setResponse(conn->_response, 200, "OK", "text/php", ctx.cgiOutput.size(),
                    new StringBodyProvider(ctx.cgiOutput));
    }
    conn->setState(Connection::SendResponse);
}
