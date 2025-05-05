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

void GetHandler::_generateErrorMsg(HttpResponse& httpResponse, std::string errorCodeToPut, std::string errorMessageToPut) {
    httpResponse.statusCode = errorCodeToPut;
    httpResponse.statusMessage = errorMessageToPut;

    // httpResponse.headers["Date"] = "";
    httpResponse.headers["Content-Type"] = "text/html; charset=UTF-8";

    httpResponse.body += "<html><body>";
    httpResponse.body += "<h1>Error " + errorCodeToPut + "</h1>";
    httpResponse.body += "<p>" + errorMessageToPut + "</p>";
    httpResponse.body += "</body></html>";

    httpResponse.headers["Content-Length"] = toString(httpResponse.body.size());
}

HttpResponse GetHandler::handle(Connection* conn, HttpRequest& request, RouteConfig& config) {
    HttpResponse httpResponse;
    if (request.headers.find("content-length") != request.headers.end() ||
        request.headers.find("transfer-encoding") != request.headers.end()) {
        _generateErrorMsg(httpResponse, "400", "Bad request");
        return httpResponse;
    }

    // std::string path = request.headers["host"] + request.uri;
}
