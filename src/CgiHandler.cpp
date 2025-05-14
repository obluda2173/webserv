#include "CgiHandler.h"

CgiHandler::CgiHandler() {}
CgiHandler::~CgiHandler() {}

std::string CgiHandler::_extractQuery(const std::string& uri) {
    const size_t pos = uri.find('?');
    return (pos != std::string::npos) ? uri.substr(pos + 1) : "";
}

std::string CgiHandler::_findInterpreter(std::map<std::string, std::string> cgiMap) {
    size_t dot = _path.rfind('.');
    if (dot == std::string::npos) return "";
    size_t end = _path.find_first_of("/?#", dot);
    std::string ext = _path.substr(dot + 1, end - dot - 1);
    std::map<std::string, std::string>::const_iterator it = cgiMap.find(ext);
    return it != cgiMap.end() ? it->second : "";
}

void CgiHandler::_setCgiEnvironment(const HttpRequest& request) {
    std::vector<std::string> env;
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
}

void CgiHandler::_parseCgiOutput(const std::string& cgiOutput, HttpResponse& resp) {
    (void)cgiOutput;
    (void)resp;
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

void CgiHandler::handle(Connection* conn, const HttpRequest& request, const RouteConfig& config) {
    HttpResponse& resp = conn->_response;

    if (!_validateAndPrepareContext(request, config, resp)) {
        conn->setState(Connection::SendResponse);
        return;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        setErrorResponse(resp, 500, "Internal Server Error", config);
        conn->setState(Connection::SendResponse);
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        setErrorResponse(resp, 500, "Internal Server Error", config);
        conn->setState(Connection::SendResponse);
        return;
    }

    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        ExecParams params;
        _prepareExecParams(request, params);

        execve(params.argv[0], 
            const_cast<char* const*>(params.argv.data()),
            const_cast<char* const*>(params.env.data()));
        exit(EXIT_FAILURE);
    } else {
        close(pipefd[1]);
        fcntl(pipefd[0], F_SETFL, O_NONBLOCK);
        conn->cgiCtx.cgiPid = pid;
        conn->cgiCtx.cgiPipeFd = pipefd[0];
        conn->cgiCtx.cgiRouteConfig = config;
        conn->setState(Connection::HandlingCgi);
    }
}

void CgiHandler::handleCgiProcess(Connection* conn) {
    CgiContext& ctx = conn->cgiCtx;

    if (ctx.cgiPipeFd != -1) {
        char buffer[4096];
        ssize_t count = read(ctx.cgiPipeFd, buffer, sizeof(buffer));
        if (count > 0) {
            ctx.cgiOutput.append(buffer, count);
        } else if (count == 0) {
            close(ctx.cgiPipeFd);
            ctx.cgiPipeFd = -1;
        } else if (count == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
            close(ctx.cgiPipeFd);
            ctx.cgiPipeFd = -1;
        }
    }

    int status;
    pid_t result = waitpid(ctx.cgiPid, &status, WNOHANG);
    if (result == -1) {
        setErrorResponse(conn->_response, 500, "Internal Server Error", ctx.cgiRouteConfig);
        conn->setState(Connection::SendResponse);
    } else if (result > 0) {
        if (ctx.cgiPipeFd != -1) {
            while (true) {
                char buffer[4096];
                ssize_t n = read(ctx.cgiPipeFd, buffer, sizeof(buffer));
                if (n > 0) ctx.cgiOutput.append(buffer, n);
                else if (n == 0) break;
                else if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                else break;
            }
            close(ctx.cgiPipeFd);
            ctx.cgiPipeFd = -1;
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            if (ctx.cgiOutput.empty()) {
                setErrorResponse(conn->_response, 500, "Internal Server Error", ctx.cgiRouteConfig);
            } else {
                // _parseCgiOutput
                setResponse(conn->_response, 200, "OK", "text/php", ctx.cgiOutput.size(), new StringBodyProvider(ctx.cgiOutput));
            }
        } else {
            setErrorResponse(conn->_response, 502, "Bad Gateway", ctx.cgiRouteConfig);
        }
        conn->setState(Connection::SendResponse);
    }
}
