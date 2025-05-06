#include "PostHandler.h"
std::map<std::string, std::string> PostHandler::mimeTypes = std::map<std::string, std::string>();

bool PostHandler::_getValidation(Connection* conn, HttpRequest& request, RouteConfig& config) {
    if (request.headers.find("content-type") == request.headers.end() || request.headers["content-type"] == "") {
		return false;
	}
	if (!strstr(request.headers["content-type"].c_str(), "boundary=")) {
		return false;
	}
	if (request.headers["content-type"].find('=') + 1 >= request.headers["content-type"].size()) {
		return false;
	}
	for (size_t it = request.headers["content-type"].find('=') + 1; it < request.headers["content-type"].size(); it++) {
		if (request.headers["content-type"][it] != ' ')
			_boundaryValue.push_back(request.headers["content-type"][it]);
	}
	if (_boundaryValue.empty()) {
		return false;
	}
	if (!_bodyBuf.size()) {
		return false;
	}
	if (request.headers.find("content-length") == request.headers.end() ||atoi(request.headers["content-length"].c_str()) > _bodyBuf.size()) {
		return false;
	}
	if (!strstr(_bodyBuf.data(), _boundaryValue.c_str())) {
		return false;
	}
	
}

void PostHandler::_setErrorResponse(HttpResponse& resp, int code, const std::string& message) {
    resp.version = "HTTP/1.1";
    resp.statusCode = code;
    resp.statusMessage = message;
    resp.contentType = "text/plain";
    resp.contentLength = message.size();
    resp.body = new StringBodyProvider(message);
}

std::string PostHandler::_normalizePath(const std::string& root, const std::string& uri) {
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

std::string PostHandler::_getMimeType(const std::string& path) {
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

void PostHandler::handle(Connection* conn, HttpRequest& request, RouteConfig& config) {
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