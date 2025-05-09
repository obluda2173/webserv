#include "DeleteHandler.h"
#include "GetHandler.h"

DeleteHandler::DeleteHandler() {}
DeleteHandler::~DeleteHandler() {}

bool DeleteHandler::_validateDeleteRequest(Connection* conn, const HttpRequest& request, const RouteConfig& config) {
    HttpResponse& resp = conn->_response;
    (void)request;
    if (_path.empty()) {
        _setErrorResponse(resp, 403, "Forbidden", config);
        return false;
    } else if (stat(_path.c_str(), &_pathStat) != 0) {
        _setErrorResponse(resp, 404, "Not Found", config);
        return false;
    } else if (!S_ISREG(_pathStat.st_mode)) {
        _setErrorResponse(resp, 400, "Bad Request: Cannot delete non-file resources", config);
        return false;
    } else if (access(_path.c_str(), W_OK) != 0) {
        _setErrorResponse(resp, 403, "Forbidden: No permission to delete", config);
        return false;
    }// For unlinkv
    return true;
}

void DeleteHandler::_setErrorResponse(HttpResponse& resp, int code, const std::string& message, const RouteConfig& config) {
    std::map<int, std::string>::const_iterator it = config.errorPage.find(code);
    if (it != config.errorPage.end()) {
        std::string errorPagePath = config.root + it->second;
        struct stat fileStat;
        if (stat(errorPagePath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
            _setResponse(resp, code, message, GetHandler::mimeTypes[".html"], fileStat.st_size, new FileBodyProvider(errorPagePath.c_str()));
            return;
        }
    }
    _setResponse(resp, code, message, "text/plain", message.size(), new StringBodyProvider(message));
}

void DeleteHandler::_setResponse(HttpResponse& resp, int statusCode, const std::string& statusMessage, const std::string& contentType, size_t contentLength, IBodyProvider* bodyProvider) {
    resp.version = "HTTP/1.1";
    resp.statusCode = statusCode;
    resp.statusMessage = statusMessage;
    resp.contentType = contentType;
    resp.contentLanguage = "en-US";
    resp.contentLength = contentLength;
    resp.body = bodyProvider;
}

std::string DeleteHandler::_normalizePath(const std::string& root, const std::string& uri) {
    (void)root;
    (void)uri;
    // return GetHandler::_normalizePath(root, uri);
    return root + uri;
}

void DeleteHandler::handle(Connection* conn, const HttpRequest& request, const RouteConfig& config) {
    _path = _normalizePath(config.root, request.uri);
    if (!_validateDeleteRequest(conn, request, config)) {
        conn->setState(Connection::SendResponse);
        return;
    }
    HttpResponse& resp = conn->_response;

    // Attempt to delete the file
    if (std::remove(_path.c_str()) == 0) {
        _setResponse(resp, 204, "No Content", "", 0, nullptr);
    } else {
        _setErrorResponse(resp, 500, "Internal Server Error", config);
    }
    conn->setState(Connection::SendResponse);
}