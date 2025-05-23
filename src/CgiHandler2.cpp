#include "CgiHandler.h"
#include "handlerUtils.h"

bool CgiHandler::_validateAndPrepareContext(const HttpRequest& request, const RouteConfig& config, Connection* conn) {
    _query = _extractQuery(request.uri);
    _scriptName = _getScriptName(config.cgi, request.uri);
    _pathInfo = _getPathInfo(config.cgi, request.uri);
    _pathTranslated = _pathInfo.empty() ? "" : (config.root + _pathInfo); 
    _path = normalizePath(config.root, _scriptName);
    _interpreter = _findInterpreter(config.cgi, request.uri);
    _serverPort = _getServerPort(conn);
    _remoteAddr = _getRemoteAddr(conn);
    if (_interpreter.empty()) {
        setErrorResponse(conn->_response, 403, "Forbidden", config);
        return false;
    }
    return validateRequest(conn->_response, request, config, _path, _pathStat);
}


void CgiHandler::_setCgiEnvironment(const HttpRequest& request) {
    _envStorage.push_back("REQUEST_METHOD=" + request.method);
    _envStorage.push_back("SCRIPT_NAME=" + _scriptName);
    _envStorage.push_back("PATH_INFO=" + _pathInfo);
    _envStorage.push_back("PATH_TRANSLATED=" + _pathTranslated);
    if (!_query.empty()) {
        _envStorage.push_back("QUERY_STRING=" + _query);
    }
    _envStorage.push_back("SERVER_NAME=" + (request.headers.count("host") ? request.headers.at("host") : "localhost"));
    _envStorage.push_back("SERVER_PORT=" + _serverPort);
    _envStorage.push_back("SERVER_PROTOCOL=" + ((request.version.empty()) ? "HTTP/1.1" : request.version));
    _envStorage.push_back("GATEWAY_INTERFACE=CGI/1.1");
    _envStorage.push_back("REMOTE_ADDR=" + _remoteAddr);
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

        std::string headerKey = _toLower(_trimWhiteSpace(line.substr(0, colon)));
        std::string headerValue = _trimWhiteSpace(line.substr(colon + 1));

        if (headerKey == "status") {
            size_t spacePos = headerValue.find(' ');
            if (spacePos != std::string::npos) {
                resp.statusCode = std::strtoul(headerValue.substr(0, spacePos).c_str(), NULL, 10);
                resp.statusMessage = headerValue.substr(spacePos + 1);
            } else {
                resp.statusCode = std::strtoul(headerValue.c_str(), NULL, 10);
                resp.statusMessage = "Unknown";
            }
        }
        else if (headerKey == "content-length") {
            resp.contentLength = std::strtoul(headerValue.c_str(), NULL, 10);
        } else if (headerKey == "content-type") {
            resp.contentType = headerValue;
        // } else {
        //     resp.headers[headerKey] = headerValue; 
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
