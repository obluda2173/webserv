#include "GetHandler.h"

GetHandler::GetHandler() {}
GetHandler::~GetHandler() {}

std::map<std::string, std::string> GetHandler::mimeTypes = std::map<std::string, std::string>();
struct MimeInitializer {
    MimeInitializer() {
        GetHandler::mimeTypes[".html"] = "text/html";
        GetHandler::mimeTypes[".htm"] = "text/html";
        GetHandler::mimeTypes[".txt"] = "text/plain";
        GetHandler::mimeTypes[".css"] = "text/css";
        GetHandler::mimeTypes[".jpg"] = "image/jpeg";
        GetHandler::mimeTypes[".jpeg"] = "image/jpeg";
        GetHandler::mimeTypes[".png"] = "image/png";
        GetHandler::mimeTypes[".gif"] = "image/gif";
        GetHandler::mimeTypes[".pdf"] = "application/pdf";
    }
};
static MimeInitializer mimeInit;

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

bool GetHandler::_validateGetRequest(Connection* conn, const HttpRequest& req, const RouteConfig& config) {
    HttpResponse& resp = conn->_response;
    _path = _normalizePath(config.root, req.uri);

    if (req.uri.empty() || _isInvalidHeader(req)) {
        _setErrorResponse(resp, 400, "Bad Request", config);
        return false;
    } else if (!_isValidPath()) {
        _setErrorResponse(resp, 403, "Forbidden", config);
        return false;
    } else if (stat(_path.c_str(), &_pathStat) != 0) {
        _setErrorResponse(resp, 404, "Not Found", config);
        return false;
    } else if (!_isAccessible() || !_isValidFileType()) {
        _setErrorResponse(resp, 403, "Forbidden", config);
        return false;
    }
    return true;
}

void GetHandler::_setErrorResponse(HttpResponse& resp, int code, const std::string& message, const RouteConfig& config) {
    std::map<int, std::string>::const_iterator it = config.errorPage.find(code);
    if (it != config.errorPage.end()) {
        std::string errorPagePath = config.root + it->second;
        struct stat fileStat;
        if (stat(errorPagePath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
            _setResponse(resp, code, message, _getMimeType(errorPagePath), fileStat.st_size, new FileBodyProvider(errorPagePath.c_str()));
            return;
        }
    }
    _setResponse(resp, code, message, "text/plain", message.size(), new StringBodyProvider(message));
}

void GetHandler::_setResponse(HttpResponse& resp, int statusCode, const std::string& statusMessage, const std::string& contentType, size_t contentLength, IBodyProvider* bodyProvider) {
    resp.version = "HTTP/1.1";
    resp.statusCode = statusCode;
    resp.statusMessage = statusMessage;
    resp.contentType = contentType;
    resp.contentLanguage = "en-US";
    resp.contentLength = contentLength;
    resp.body = bodyProvider;
}

// handle URI-encoded characters
// handle query parameters
std::string GetHandler::_normalizePath(const std::string& root, const std::string& uri) {
    std::vector<std::string> segments;
    std::string segment;
    std::istringstream iss(uri);
    
    while (std::getline(iss, segment, '/')) {
        if (segment.empty() || segment == ".") continue;
        if (segment == "..") {
            if (!segments.empty()) segments.pop_back();
        } else {
            segments.push_back(segment);
        }
    }

    std::ostringstream oss;
    oss << root;
    for (size_t i = 0; i < segments.size(); ++i) {
        oss << '/' << segments[i];
    }
    std::string result = oss.str();

    if (uri == "/") {
        result += "/";
    }
    return (result.find(root) == 0) ? result : "";
}

std::string GetHandler::_getMimeType(const std::string& path) {
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
    if (!_validateGetRequest(conn, request, config)) {
        conn->setState(Connection::SendResponse);
        return;
    }

    HttpResponse& resp = conn->_response;
    ScopedBodyProvider body;

    if (S_ISREG(_pathStat.st_mode)) {
        body.ptr = new FileBodyProvider(_path.c_str());
        _setResponse(resp, 200, "OK", _getMimeType(_path), _pathStat.st_size, body.ptr);
        body.ptr = 0;
    } else if (S_ISDIR(_pathStat.st_mode)) {
        if (!config.index.empty()) {
            for (size_t i = 0; i < config.index.size(); ++i) {
                std::string indexPath = _path + config.index[i];
                if (stat(indexPath.c_str(), &_pathStat) == 0 && S_ISREG(_pathStat.st_mode)) {
                    body.ptr = new FileBodyProvider(indexPath.c_str());
                    _setResponse(resp, 200, "OK", _getMimeType(indexPath), _pathStat.st_size, body.ptr);
                    body.ptr = 0;
                    break;
                }
            }
        }
        if (resp.statusCode != 200 && config.autoindex) {
            std::string listing;
            if (_getDirectoryListing(_path, request.uri, listing)) {
                body.ptr = new StringBodyProvider(listing);
                _setResponse(resp, 200, "OK", "text/html", listing.size(), body.ptr);
                body.ptr = 0;
            }
        }
        if (resp.statusCode != 200) {
            _setErrorResponse(resp, 403, "Forbidden", config);
        }
    }
    conn->setState(Connection::SendResponse);
}