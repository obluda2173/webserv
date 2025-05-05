#include "GetHandler.h"

bool GetHandler::_getValidation(Connection* conn, HttpRequest& request, RouteConfig& config) {
    if (request.headers.find("content-length") != request.headers.end() ||               // no body in GET method
        request.headers.find("transfer-encoding") != request.headers.end()) {
        return false;
    } else if (stat(_path.c_str(), &_pathStat) != 0) {
        return false;
    }
    // check whether GET is applicable to the provided allow_methods (disscuss with Mr Kay and Mr Yao)
}

void GetHandler::handle(Connection* conn, HttpRequest& request, RouteConfig& config) {
    HttpResponse& resp = conn->_response;
    _path = config.root + request.uri;

    if (!_getValidation(conn, request, config)) {
        // error message send
    }
    if (S_ISREG(_pathStat.st_mode)) {                       // file
        resp.contentLength = _pathStat.st_size;
        resp.body = new FileBodyProvider(_path.c_str());
        resp.statusCode = 200;
        resp.statusMessage = "OK";
        resp.contentType = "text/html";
        resp.contentLanguage = "en-US";
        resp.version = "HTTP/1.1";
        conn->setState(Connection::SendResponse);
        return;
    } else if (S_ISDIR(_pathStat.st_mode)) {               // directory
        if (!config.index.empty()) {
            for (std::vector<std::string>::const_iterator it = config.index.begin(); it != config.index.end(); ++it) {
                std::string indexPath = _path + *it;
                if (stat((indexPath).c_str(), &_pathStat) == 0) {
                    resp.contentLength = _pathStat.st_size;
                    resp.body = new FileBodyProvider(indexPath.c_str());
                    resp.statusCode = 200;
                    resp.statusMessage = "OK";
                    resp.contentType = "text/html";
                    resp.contentLanguage = "en-US";
                    resp.version = "HTTP/1.1";
                    conn->setState(Connection::SendResponse);
                    return;
                }
            }
        } 
        if (config.autoindex) {
            // if autoindex is on, generate directory listing
        } else {
            // error message with 403 Forbidden 
        }
    }
}
