#include "GetHandler.h"

GetHandler::GetHandler() {}
GetHandler::~GetHandler() {}

bool GetHandler::_isInvalidHeader(const HttpRequest& req) const {
    return req.headers.count("content-length") || req.headers.count("transfer-encoding");
}

bool GetHandler::_isValidPath() const {
    return !_path.empty() && _path.length() <= MAX_PATH_LENGTH;
}

bool GetHandler::_isAccessible() const {
    return access(_path.c_str(), R_OK) == 0;
}

bool GetHandler::_isValidFileType() const {
    return S_ISREG(_pathStat.st_mode) || S_ISDIR(_pathStat.st_mode);
}

bool GetHandler::_validateGetRequest(HttpResponse& resp, const HttpRequest& req, const RouteConfig& config) {
    if (req.uri.empty() || _isInvalidHeader(req)) {
        setErrorResponse(resp, 400, "Bad Request", config);
        return false;
    } else if (!_isValidPath()) {
        setErrorResponse(resp, 403, "Forbidden", config);
        return false;
    } else if (stat(_path.c_str(), &_pathStat) != 0) {
        setErrorResponse(resp, 404, "Not Found", config);
        return false;
    } else if (!_isAccessible() || !_isValidFileType()) {
        setErrorResponse(resp, 403, "Forbidden", config);
        return false;
    }
    return true;
}

bool GetHandler::_getDirectoryListing(const std::string& dirPath, const std::string& requestPath, std::string& outListing) {
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) {
        return false;
    }

    std::vector<std::string> entries;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name == "." || name == "..")
            continue;
        std::string fullPath = dirPath + "/" + name;
        struct stat statbuf;
        if (stat(fullPath.c_str(), &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
            name += "/";
        }
        entries.push_back(name);
    }
    closedir(dir);

    std::ostringstream html;
    html << "<!DOCTYPE html>\n"
         << "<html><head><title>Index of " << requestPath << "</title></head><body>\n"
         << "<h1>Index of " << requestPath << "</h1><ul>\n";
    for (size_t i = 0; i < entries.size(); ++i) {
        html << "<li><a href=\"" << entries[i] << "\">" << entries[i] << "</a></li>\n";
    }
    html << "</ul></body></html>\n";
    outListing = html.str();
    return true;
}

void GetHandler::handle(Connection* conn, const HttpRequest& request, const RouteConfig& config) {
    HttpResponse& resp = conn->_response;
    _path = normalizePath(config.root, request.uri);
    if (!_validateGetRequest(resp, request, config)) { // basic validation
        conn->setState(Connection::SendResponse);
        return;
    }

    if (S_ISREG(_pathStat.st_mode)) { // if file
        setResponse(resp, 200, "OK", getMimeType(_path), _pathStat.st_size, new FileBodyProvider(_path.c_str()));
        conn->setState(Connection::SendResponse);
        return;
    } else if (S_ISDIR(_pathStat.st_mode)) { // if directory
        if (!config.index.empty()) {
            for (size_t i = 0; i < config.index.size(); ++i) { // serve existing index file from "index" directive (confg)
                std::string indexPath = _path + config.index[i];
                if (stat(indexPath.c_str(), &_pathStat) == 0 && S_ISREG(_pathStat.st_mode)) {
                    setResponse(resp, 200, "OK", getMimeType(indexPath), _pathStat.st_size, new FileBodyProvider(indexPath.c_str()));
                    conn->setState(Connection::SendResponse);
                    return;
                }
            }
        }
        if (config.autoindex) { // if no existing index file was found
            std::string listing;
            if (_getDirectoryListing(_path, request.uri, listing)) { // serve directory listing
                setResponse(resp, 200, "OK", "text/html", listing.size(), new StringBodyProvider(listing));
                conn->setState(Connection::SendResponse);
                return;
            }
        }
    }
    setErrorResponse(resp, 403, "Forbidden", config); // no need to explain
    conn->setState(Connection::SendResponse);
    return;
}