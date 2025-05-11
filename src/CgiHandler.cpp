#include "CgiHandler.h"

CgiHandler::CgiHandler() {}
CgiHandler::~CgiHandler() {}

void CgiHandler::_setCgiEnvironment(Connection& conn, const HttpRequest& request) {
    (void)conn;
    (void)request;
}

std::string CgiHandler::_executeCgiScript() {
    return "";
}

void CgiHandler::_parseCgiOutput(const std::string& cgiOutput, HttpResponse& resp) {
    (void)cgiOutput;
    (void)resp;
}

void CgiHandler::handle(Connection* conn, const HttpRequest& request, const RouteConfig& config) {
    HttpResponse& resp = conn->_response;
    
    // validation
    if (!validateRequest(resp, request, config, _path, _pathStat)) {
        conn->setState(Connection::SendResponse);
        return;
    }

    // environment
    _setCgiEnvironment(*conn, request);

    // execute  script
    std::string cgiOutput = _executeCgiScript();
    if (cgiOutput.empty()) {
        setErrorResponse(resp, 500, "Internal Server Error", config);
        conn->setState(Connection::SendResponse);
        return;
    }

    // 4. Parse CGI output
    _parseCgiOutput(cgiOutput, resp);
    conn->setState(Connection::SendResponse);
}
