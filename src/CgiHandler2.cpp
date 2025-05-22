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

std::string CgiHandler::_toUpper(const std::string& str) {
    std::string result = str;
    for (char* ptr = &result[0]; ptr < &result[0] + result.size(); ++ptr) {
        *ptr = static_cast< char >(toupper(*ptr));
    }
    return result;
}

void CgiHandler::_replace(std::string& str, char what, char with) {
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == what) {
            str[i] = with;
        }
    }
}

void CgiHandler::_setCgiEnvironment(const HttpRequest& request) {
    _envStorage.push_back("REQUEST_METHOD=" + request.method);
    _envStorage.push_back("SCRIPT_NAME=" + _path.substr(_path.find_last_of("/")));
    _envStorage.push_back("PATH_INFO=" + _path);
    _envStorage.push_back("PATH_TRANSLATED=" + _path); // "PATH_TRANSLATED=" + config.root + _path
    if (!_query.empty()) {
        _envStorage.push_back("QUERY_STRING=" + _query);
    }
    _envStorage.push_back("SERVER_NAME=" + (request.headers.count("host") ? request.headers.at("host") : ""));
    // _envStorage.push_back("SERVER_PORT=" + std::to_string(serverConfig.listen.begin()->second));
    _envStorage.push_back("SERVER_PROTOCOL=" + request.version);
    _envStorage.push_back("GATEWAY_INTERFACE=CGI/1.1");
    _envStorage.push_back("REQUEST_URI=" + request.uri);
    // REMOTE_ADDR
    // REMOTE_HOST
    if (request.headers.count("content-length")) {
        _envStorage.push_back("CONTENT_LENGTH=" + request.headers.at("content-length"));
    }
    if (request.headers.count("content-type")) {
        _envStorage.push_back("CONTENT_TYPE=" + request.headers.at("content-type"));
    }
    for (std::map< std::string, std::string >::const_iterator it = request.headers.begin(); it != request.headers.end(); ++it) {
        if (it->first == "content-length" || it->first == "content-type" || it->first == "authorization") {
            continue;
        }
        std::string varName = "HTTP_" + _toUpper(it->first);
        _replace(varName, '-', '_');
        _envStorage.push_back(varName + "=" + it->second);
    }
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

void CgiHandler::_setupChildProcess(int pipeStdin[2], int pipeStdout[2]) {
    close(pipeStdin[1]);
    close(pipeStdout[0]);
    dup2(pipeStdin[0], STDIN_FILENO);
    dup2(pipeStdout[1], STDOUT_FILENO);
    close(pipeStdin[0]);
    close(pipeStdout[1]);
}

void CgiHandler::_setupParentProcess(Connection* conn, int pipeStdin[2], int pipeStdout[2], pid_t pid, const RouteConfig& config) {
    (void)config;
    close(pipeStdin[0]);
    close(pipeStdout[1]);
    conn->cgiCtx.cgiPipeStdin = pipeStdin[1];
    conn->cgiCtx.cgiPipeStdout = pipeStdout[0];
    conn->cgiCtx.cgiPid = pid;
    conn->cgiCtx.state = CgiContext::WritingStdin;
    fcntl(conn->cgiCtx.cgiPipeStdin, F_SETFL, O_NONBLOCK);
    fcntl(conn->cgiCtx.cgiPipeStdout, F_SETFL, O_NONBLOCK);
}

std::string CgiHandler::_trimWhiteSpace(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (std::string::npos == first)
        return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}

void CgiHandler::_cgiResponseSetup(const std::string& cgiOutput, HttpResponse& resp) {
    resp.statusCode = 200;
    resp.statusMessage = "OK";
    resp.contentType = mimeTypes[".html"];
    resp.contentLength = 0;

    size_t separatorSize = 4;
    size_t headerEnd = cgiOutput.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        headerEnd = cgiOutput.find("\n\n");
        separatorSize = 2;
        if (headerEnd == std::string::npos) {
            resp.body = new StringBodyProvider(cgiOutput);
            return;
        }
    }

    const std::string headersStr = cgiOutput.substr(0, headerEnd);
    const std::string body = cgiOutput.substr(headerEnd + separatorSize);

    size_t strStart = 0;
    while (strStart < headersStr.length()) {
        size_t strEnd = headersStr.find("\r\n", strStart);
        if (strEnd == std::string::npos)
            strEnd = headersStr.find('\n', strStart);
        if (strEnd == std::string::npos)
            strEnd = headersStr.length();

        std::string line = headersStr.substr(strStart, strEnd - strStart);
        strStart = (strEnd != headersStr.length()) ? strEnd + ((headersStr[strEnd] == '\r') ? 2 : 1) : strEnd;

        if (line.empty())
            continue;

        size_t colon = line.find(':');
        if (colon == std::string::npos)
            continue;

        std::string headerKey = _trimWhiteSpace(line.substr(0, colon));
        std::string headerValue = _trimWhiteSpace(line.substr(colon + 1));
        if (headerKey == "Content-Length") {
            resp.contentLength = std::stoi(headerValue);
        } else if (headerKey == "Content-Type") {
            resp.contentType = headerValue;
        }
    }
    resp.body = new StringBodyProvider(body);
}

bool CgiHandler::_readPipeData(CgiContext& cgiCtx, bool drain) {
    if (cgiCtx.cgiPipeStdout == -1) {
        return false;
    }

    do {
        char buffer[4096];
        const ssize_t count = read(cgiCtx.cgiPipeStdout, buffer, sizeof(buffer));
        if (count > 0) {
            cgiCtx.cgiOutput.append(buffer, count);
        } else {
            if (count == 0 || (count == -1 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                close(cgiCtx.cgiPipeStdout);
                cgiCtx.cgiPipeStdout = -1;

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
    if (ctx.cgiPipeStdout != -1) {
        _readPipeData(ctx, true);
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        setErrorResponse(conn->_response, 502, "Bad Gateway", ctx.cgiRouteConfig);
    } else if (ctx.cgiOutput.empty()) {
        setErrorResponse(conn->_response, 500, "Internal Error", ctx.cgiRouteConfig);
    } else {
        _cgiResponseSetup(ctx.cgiOutput, conn->_response);
    }
    conn->setState(Connection::SendResponse);
}
