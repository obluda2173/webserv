#include "GetHandler.h"

std::map<std::string, std::string> GetHandler::mimeTypes = std::map<std::string, std::string>();

bool GetHandler::_getValidation(Connection* conn, HttpRequest& request, RouteConfig& config) {
    if (request.headers.find("content-length") != request.headers.end() ||
        request.headers.find("transfer-encoding") != request.headers.end()) {
        _setErrorResponse(conn->_response, 400, "Bad Request: GET requests should not have a body");
        return false;
    }
    if (stat(_path.c_str(), &_pathStat) != 0) {
        return false;
    }
    return true;
}

void GetHandler::_setGoodResponse(HttpResponse& resp, std::string mimeType, int statusCode, size_t fileSize, IBodyProvider* bodyProvider) {      // this should be a private function
    resp.contentLength = fileSize;
    resp.body = bodyProvider;
    resp.statusCode = 200;
    resp.statusMessage = "OK";
    resp.contentType = mimeType;
    resp.contentLanguage = "en-US";
    resp.version = "HTTP/1.1";
}

void GetHandler::_setErrorResponse(HttpResponse& resp, int code, const std::string& message) {
    resp.version = "HTTP/1.1";
    resp.statusCode = code;
    resp.statusMessage = message;
    resp.contentType = "text/plain";
    resp.contentLength = message.size();
    resp.body = new StringBodyProvider(message);
}

std::string GetHandler::_normalizePath(const std::string& root, const std::string& uri) {
    std::string fullPath = root + uri;
    std::string normalized = "";
    std::vector<std::string> segments;

    // Split path by '/'
    std::string segment = "";
    for (std::string::const_iterator it = fullPath.begin(); it != fullPath.end(); ++it) {
        if (*it == '/') {
            if (!segment.empty()) {
                segments.push_back(segment);
                segment = "";
            }
        } else {
            segment += *it;
        }
    }
    if (!segment.empty()) {
        segments.push_back(segment);
    }

    // Canonicalize path
    std::vector<std::string> stack;
    for (std::vector<std::string>::const_iterator it = segments.begin(); it != segments.end(); ++it) {
        if (*it == "..") {
            if (!stack.empty()) {
                stack.pop_back();
            }
        } else if (*it != "." && !it->empty()) {
            stack.push_back(*it);
        }
    }

    // Rebuild path
    if (stack.empty()) {
        normalized = root + "/";
    } else {
        normalized = root;
        for (std::vector<std::string>::const_iterator it = stack.begin(); it != stack.end(); ++it) {
            normalized += "/" + *it;
        }
    }

    // Ensure the path stays within root
    if (normalized.find(root) != 0) {
        return "";
    }
    return normalized;
}

std::string GetHandler::_getMimeType(const std::string& path) {
    if (mimeTypes.empty()) {
        mimeTypes[".html"] = "text/html";
        mimeTypes[".txt"] = "text/plain";
        mimeTypes[".jpg"] = "image/jpeg";
        mimeTypes[".png"] = "image/png";
    }
    std::string ext = "";
    std::string::size_type pos = path.rfind('.');
    if (pos != std::string::npos) {
        ext = path.substr(pos);
    }
    std::map<std::string, std::string>::const_iterator it = mimeTypes.find(ext);
    if (it != mimeTypes.end()) {
        return it->second;
    }
    return "application/octet-stream";
}

std::string GetHandler::_getDirectoryListing(const std::string& dirPath, const std::string& requestPath) {
    DIR* dir = opendir(dirPath.c_str());

    std::vector<std::string> entries;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;

        // Skip . and ..
        if (name == "." || name == "..")
            continue;

        std::string fullPath = dirPath + "/" + name;
        struct stat statbuf;
        if (stat(fullPath.c_str(), &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
            name += "/";  // Append slash to folders
        }

        entries.push_back(name);
    }
    closedir(dir);

    // Now build HTML
    std::ostringstream html;
    html << "<!DOCTYPE html>\n"
         << "<html><head><title>Index of " << requestPath << "</title></head><body>\n"
         << "<h1>Index of " << requestPath << "</h1><ul>\n";

    for (size_t i = 0; i < entries.size(); ++i) {
        html << "<li><a href=\"" << entries[i] << "\">" << entries[i] << "</a></li>\n";
    }

    html << "</ul></body></html>\n";
    return html.str();
}

void GetHandler::handleGoodResponce(Connection* conn, std::string pathToPut) {      // this should be a private function
    HttpResponse& resp = conn->_response;
    resp.contentLength = _pathStat.st_size;
    resp.body = new FileBodyProvider(pathToPut.c_str());
    resp.statusCode = 200;
    resp.statusMessage = "OK";
    resp.contentType = "text/html";
    resp.contentLanguage = "en-US";
    resp.version = "HTTP/1.1";
    conn->setState(Connection::SendResponse);
}

void GetHandler::handleBadResponce(Connection* conn, int statusCode, std::string statusMessage) {       // this should be a private function
    HttpResponse& resp = conn->_response;
    resp.statusCode = statusCode;
    resp.statusMessage = statusMessage;
    resp.contentType = "text/html";
    resp.contentLanguage = "en-US";
    resp.version = "HTTP/1.1";
    // build a tiny HTML page that includes the code & message
    std::string tempBody =
        "<!DOCTYPE html>\r\n"
        "<html lang=\"en\">\r\n"
        "<head>\r\n"
        "  <meta charset=\"UTF-8\">\r\n"
        "  <title>" + std::to_string(statusCode) + " " + statusMessage + "</title>\r\n"     // we need our own to_string here
        "</head>\r\n"
        "<body>\r\n"
        "  <h1>" + std::to_string(statusCode) + " " + statusMessage + "</h1>\r\n"           // we need our own to_string here
        "  <p>" + statusMessage + ".</p>\r\n"
        "</body>\r\n"
        "</html>\r\n";
    resp.body = new StringBodyProvider(tempBody);
    resp.contentLength = resp.body.size();
    conn->setState(Connection::SendResponse);
}

#include <dirent.h>

std::string generateDirectoryIndexHtml(const std::string& dirPath, const std::string& requestPath) {
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) {         // we don't need this if at all
        return "<html><body><h1>403 Forbidden</h1></body></html>";
    }

    std::vector<std::string> entries;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;

        // Skip . and ..
        if (name == "." || name == "..")
            continue;

        std::string fullPath = dirPath + "/" + name;
        struct stat statbuf;
        if (stat(fullPath.c_str(), &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
            name += "/";  // Append slash to folders
        }

        entries.push_back(name);
    }
    closedir(dir);

    // Now build HTML
    std::ostringstream html;
    html << "<!DOCTYPE html>\n"
         << "<html><head><title>Index of " << requestPath << "</title></head><body>\n"
         << "<h1>Index of " << requestPath << "</h1><ul>\n";

    for (size_t i = 0; i < entries.size(); ++i) {
        html << "<li><a href=\"" << entries[i] << "\">" << entries[i] << "</a></li>\n";
    }

    html << "</ul></body></html>\n";
    return html.str();
}

void GetHandler::handleAutoIndex(Connection* conn, std::string pathToPut) {      // this should be a private function
    HttpResponse& resp = conn->_response;
    std::string tempBody = generateDirectoryIndexHtml(pathToPut, "");
    resp.contentLength = tempBody.size();
    resp.body = new StringBodyProvider(tempBody);
    resp.statusCode = 200;
    resp.statusMessage = "OK";
    resp.contentType = "text/html";
    resp.contentLanguage = "en-US";
    resp.version = "HTTP/1.1";
    conn->setState(Connection::SendResponse);
}

void GetHandler::handle(Connection* conn, HttpRequest& request, RouteConfig& config) {
    HttpResponse& resp = conn->_response;
    std::string uri = request.uri;
    _path = _normalizePath(config.root, uri);

    if (_path.empty()) {
        _setErrorResponse(resp, 403, "Forbidden");
        conn->setState(Connection::SendResponse);
        return;
    }

    if (!_getValidation(conn, request, config)) {
        _setErrorResponse(resp, 404, "Not Found");
        conn->setState(Connection::SendResponse);
        return;
    }

    if (S_ISREG(_pathStat.st_mode)) {
        _setGoodResponse(resp, _getMimeType(_path), 200, _pathStat.st_size, new FileBodyProvider(_path.c_str()));
        conn->setState(Connection::SendResponse);
    } else if (S_ISDIR(_pathStat.st_mode)) {
        if (!config.index.empty()) {
            for (std::vector<std::string>::const_iterator it = config.index.begin(); it != config.index.end(); ++it) {
                std::string indexPath = _path + *it;
                if (stat(indexPath.c_str(), &_pathStat) == 0 && S_ISREG(_pathStat.st_mode)) {
                    _setGoodResponse(resp, _getMimeType(indexPath), 200, _pathStat.st_size, new FileBodyProvider(indexPath.c_str()));
                    conn->setState(Connection::SendResponse);
                    return;
                }
            }
        }
        if (config.autoindex) {
            std::string listing = _getDirectoryListing(_path, uri);
            _setGoodResponse(resp, "text/html", 200, listing.size(), new StringBodyProvider(listing));
            conn->setState(Connection::SendResponse);
        } else {
            _setErrorResponse(resp, 403, "Forbidden");
            conn->setState(Connection::SendResponse);
        }
    } else {
        _setErrorResponse(resp, 404, "Not Found");
        conn->setState(Connection::SendResponse);
    }
}