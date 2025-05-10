#include "DeleteHandler.h"
#include "GetHandler.h"

DeleteHandler::DeleteHandler() {}
DeleteHandler::~DeleteHandler() {}

bool DeleteHandler::_validateDeleteRequest(Connection* conn, const HttpRequest& request, const RouteConfig& config) {
    HttpResponse& resp = conn->_response;
    (void)request;
    if (_path.empty()) {
        setErrorResponse(resp, 403, "Forbidden", config);
        return false;
    } else if (stat(_path.c_str(), &_pathStat) != 0) {
        setErrorResponse(resp, 404, "Not Found", config);
        return false;
    } else if (!S_ISREG(_pathStat.st_mode)) {
        setErrorResponse(resp, 400, "Bad Request: Cannot delete non-file resources", config);
        return false;
    } else if (access(_path.c_str(), W_OK) != 0) {
        setErrorResponse(resp, 403, "Forbidden: No permission to delete", config);
        return false;
    }// For unlinkv
    return true;
}

void DeleteHandler::handle(Connection* conn, const HttpRequest& request, const RouteConfig& config) {
    _path = normalizePath(config.root, request.uri);
    if (!_validateDeleteRequest(conn, request, config)) {
        conn->setState(Connection::SendResponse);
        return;
    }
    HttpResponse& resp = conn->_response;

    // Attempt to delete the file
    if (std::remove(_path.c_str()) == 0) {
        setResponse(resp, 204, "No Content", "", 0, nullptr);
    } else {
        setErrorResponse(resp, 500, "Internal Server Error", config);
    }
    conn->setState(Connection::SendResponse);
}