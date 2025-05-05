#include "GetHandler.h"

/*
HTTP/1.1 400 Bad Request
Date: Tue, 04 May 2025 12:00:00 GMT
Content-Type: text/html; charset=UTF-8
Content-Length: 194
*/

std::string toString(size_t value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

bool GetHandler::_validation(Connection* conn, HttpRequest& request, RouteConfig& config) {
    if (request.headers.find("content-length") != request.headers.end() ||
        request.headers.find("transfer-encoding") != request.headers.end()) {
        return false;
    }
}

void GetHandler::handle(Connection* conn, HttpRequest& request, RouteConfig& config) {
    HttpResponse& resp = conn->_response;
    
    if (GetHandler::_validation(conn, request, config)) {
        // change the state into bad request then return.
    }

    // std::string path = request.headers["host"] + request.uri;
}
