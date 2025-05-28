#include "CgiHandler.h"
#include "handlerUtils.h"

bool CgiHandler::_validateAndPrepareContext(const HttpRequest& request, const RouteConfig& config, Connection* conn) {
    _interpreter = findInterpreter(config.cgi, request.uri);
    if (_interpreter.empty()) {
        setErrorResponse(conn->_response, 403, config);
        return false;
    }
    _path = config.root + getScriptName(config.cgi, request.uri);
    return validateRequest(conn->_response, request, config, _path, _pathStat);
}

void CgiHandler::_setCgiEnvironment(const HttpRequest& request, const RouteConfig& config, Connection* conn) {
    std::string scriptName = getScriptName(config.cgi, request.uri);
    std::string pathInfo = getPathInfo(config.cgi, request.uri);

    _envStorage.push_back("GATEWAY_INTERFACE=CGI/1.1");
    _envStorage.push_back("PATH_INFO=" + pathInfo);
    _envStorage.push_back("QUERY_STRING=" + request.query);
    _envStorage.push_back("SCRIPT_NAME=" + scriptName);
    _envStorage.push_back("SERVER_PORT=" + getServerPort(conn));
    _envStorage.push_back("REMOTE_ADDR=" + getRemoteAddr(conn));
    _envStorage.push_back("REQUEST_METHOD=" + request.method);
    _envStorage.push_back("PATH_TRANSLATED=" + (pathInfo.empty() ? "" : config.root + pathInfo));
    _envStorage.push_back("SERVER_PROTOCOL=" + ((request.version.empty()) ? "HTTP/1.1" : request.version));
    _envStorage.push_back("SERVER_NAME=" + (request.headers.count("host") ? request.headers.at("host") : "localhost"));
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
        std::string varName = "HTTP_" + toUpper(it->first);
        replace(varName, '-', '_');
        _envStorage.push_back(varName + "=" + it->second);
    }
}

void CgiHandler::_setupChildProcess(int pipeStdin[2], int pipeStdout[2], Connection* conn, const HttpRequest& request, const RouteConfig& config) {
    close(pipeStdin[1]);
    close(pipeStdout[0]);
    dup2(pipeStdin[0], STDIN_FILENO);
    dup2(pipeStdout[1], STDOUT_FILENO);
    close(pipeStdin[0]);
    close(pipeStdout[1]);

    // std::ofstream outfile ("test.txt");
    _execParams.argv.push_back(_interpreter.c_str());
    _execParams.argv.push_back(_path.c_str());
    _execParams.argv.push_back(NULL);
    _setCgiEnvironment(request, config, conn);
    for (size_t i = 0; i < _envStorage.size(); i++) {
        _execParams.env.push_back(_envStorage[i].c_str());
        // outfile << _envStorage[i] << std::endl;
    }
    // outfile.close();
    _execParams.env.push_back(NULL);
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

void CgiHandler::_cgiResponseSetup(const std::string& cgiOutput, HttpResponse& resp, RouteConfig& config) {
    resp.statusCode = 200;
    resp.statusMessage = "OK";
    resp.contentType = "";
    resp.contentLength = 0;

    size_t separatorSize = 4;
    size_t headerEnd = cgiOutput.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        headerEnd = cgiOutput.find("\n\n");
        separatorSize = 2;
        if (headerEnd == std::string::npos) {
            setErrorResponse(resp, 500, config);
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

        std::string headerKey = toLower(trimWhiteSpace(line.substr(0, colon)));
        std::string headerValue = trimWhiteSpace(line.substr(colon + 1));

        if (headerKey == "status") {
            size_t spacePos = headerValue.find(' ');
            std::string code = headerValue.substr(0, spacePos);
            std::string phrase = headerValue.substr(spacePos + 1);
            if (spacePos != std::string::npos) {
                if (code.length() == 3 && std::isdigit(code[0]) && std::isdigit(code[1]) && std::isdigit(code[2])) {
                    resp.statusCode = std::strtoul(code.c_str(), NULL, 10);
                    resp.statusMessage = phrase;
                } else {
                    setErrorResponse(resp, 500, config);
                    return;
                }
            } else {
                setErrorResponse(resp, 500, config);
                return;
            }
        }
        else if (headerKey == "content-length") {
            resp.contentLength = std::strtoul(headerValue.c_str(), NULL, 10);
            if (resp.contentLength != body.size()) {
                setErrorResponse(resp, 500, config);
                return;
            }
        } else if (headerKey == "content-type") {
            resp.contentType = headerValue;
        } else {
            resp.headers[headerKey] = headerValue; 
        }
    }
    if (resp.contentType == "") {
        setErrorResponse(resp, 500, config);
        return;
    }
    if (resp.contentLength == 0) {
        resp.contentLength = body.size();
    }
    resp.body = new StringBodyProvider(body);
}

bool CgiHandler::_readPipeData(CgiContext& cgiCtx, bool drain) {
    if (cgiCtx.cgiPipeStdout == -1) {
        return true;
    }

    do {
        char buffer[4096];
        const ssize_t count = read(cgiCtx.cgiPipeStdout, buffer, sizeof(buffer));
        
        if (count > 0) {
            cgiCtx.cgiOutput.append(buffer, count);
            return true;
        } else {
            if (count == 0 || (count == -1 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                close(cgiCtx.cgiPipeStdout);
                cgiCtx.cgiPipeStdout = -1;
                if (count == -1) {
                    return false;
                }
                return true;
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
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0 || WIFSIGNALED(status)) {
        setErrorResponse(conn->_response, 502, ctx.cgiRouteConfig);
    } else if (ctx.cgiOutput.empty()) {
        setErrorResponse(conn->_response, 500, ctx.cgiRouteConfig);
    } else {
        _cgiResponseSetup(ctx.cgiOutput, conn->_response, ctx.cgiRouteConfig);
    }
    conn->setState(Connection::SendResponse);
}
