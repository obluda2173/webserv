#include "DeleteHandler.h"

DeleteHandler::DeleteHandler() {}
DeleteHandler::~DeleteHandler() {}

bool DeleteHandler::_deleteResource() {
    if (S_ISDIR(_pathStat.st_mode)) {
        return rmdir(_path.c_str()) == 0;
    }
    return std::remove(_path.c_str()) == 0;
}

void DeleteHandler::handle(Connection* conn, const HttpRequest& request, const RouteConfig& config) {
    HttpResponse& resp = conn->_response;
    _path = normalizePath(config.root, request.uri);
    if (!validateRequest(resp, request, config, "DELETE")) {
        conn->setState(Connection::SendResponse);
        return;
    }

    if (_deleteResource()) {
        setResponse(resp, 204, "No Content", "", 0, nullptr);
    } else {
        setErrorResponse(resp, 500, "Internal Server Error", config);
    }
    conn->setState(Connection::SendResponse);
}
