#include "PostHandler.h"
std::map<std::string, std::string> PostHandler::mimeTypes = std::map<std::string, std::string>();

void PostHandler::divideBody(const std::vector<char>& bodyBuf, const std::string& boundary) {
    std::string dashBoundary = "--" + boundary;
    std::string endBoundary = dashBoundary + "--";

    std::vector<char> boundaryVec(dashBoundary.begin(), dashBoundary.end());
    std::vector<char> endBoundaryVec(endBoundary.begin(), endBoundary.end());

    std::vector<char>::const_iterator pos = bodyBuf.begin();
    std::vector<char>::const_iterator bodyEnd = bodyBuf.end();

    while (true) {
        // Search for next boundary
        std::vector<char>::const_iterator boundaryPos = std::search(pos, bodyEnd, boundaryVec.begin(), boundaryVec.end());
        if (boundaryPos == bodyEnd)
            break;

        // Extract part if non-empty
        if (pos != boundaryPos) {
            std::vector<char> part(pos, boundaryPos);

            // Strip trailing CRLF
            if (part.size() >= 2 &&
                part[part.size() - 2] == '\r' &&
                part[part.size() - 1] == '\n') {
                part.erase(part.end() - 2, part.end());
            }

            _parts.push_back(part);
        }

        // Move past boundary
        pos = boundaryPos + boundaryVec.size();

        // Optional CRLF after boundary
        if (pos + 1 < bodyEnd && *pos == '\r' && *(pos + 1) == '\n') {
            pos += 2;
        }

        // Check for final boundary
        std::vector<char>::const_iterator endCheckPos = boundaryPos;
        std::vector<char>::const_iterator finalCheck = std::search(endCheckPos, bodyEnd,
                                                                   endBoundaryVec.begin(), endBoundaryVec.end());
        if (finalCheck == boundaryPos) {
            break; // reached final boundary
        }
    }
}

bool PostHandler::putIntoStruct(void) {
    // now put everything into the vector struct
    for (size_t i = 0; i < _parts.size(); i++)
    {
        inBoundary tempSubBody;
        std::string token = "filename=";
        std::vector<char> tokenVec(token.begin(), token.end());
        std::vector<char>::const_iterator tokenPos = std::search(_parts[i].begin(), _parts[i].end(), tokenVec.begin(), tokenVec.end());
        if (tokenPos == _parts[i].end()) {
            return false;
        }
        tokenPos += 9;
        if (tokenPos != _parts[i].end() && *tokenPos == '"')
            ++tokenPos; // Skip opening quote
        while (tokenPos != _parts[i].end() && *tokenPos != '"')
        {
            tempSubBody.fileName.push_back(*tokenPos);
            tokenPos++;
        }
        token = "\r\n\r\n";
        tokenVec = std::vector<char>(token.begin(), token.end());
        tokenPos = std::search(_parts[i].begin(), _parts[i].end(), tokenVec.begin(), tokenVec.end());
        if (tokenPos == _parts[i].end()) {
            return false;
        }
        tokenPos += 4;
        while (tokenPos != _parts[i].end())
        {
            tempSubBody.bodyContent.push_back(*tokenPos);
            tokenPos++;
        }
        // Remove possible trailing \r\n
        if (tempSubBody.bodyContent.size() >= 2 &&
            tempSubBody.bodyContent[tempSubBody.bodyContent.size() - 2] == '\r' &&
            tempSubBody.bodyContent[tempSubBody.bodyContent.size() - 1] == '\n') {
                tempSubBody.bodyContent.erase(tempSubBody.bodyContent.end() - 2, tempSubBody.bodyContent.end());
        }

        _subBody.push_back(tempSubBody);
    }
    
    return true;
}

bool PostHandler::_getValidation(Connection* conn, HttpRequest& request, RouteConfig& config) {
    if (request.uri.find("..") != std::string::npos) {      // not allow /../ appear
        return false;
    }
    if (request.headers.find("content-type") == request.headers.end() || request.headers["content-type"] == "") {       // no content-type or no value
		return false;
	}
	if (!strstr(request.headers["content-type"].c_str(), "boundary=")) {        // no boundary
		return false;
	}
	if (request.headers["content-type"].find('=') + 1 >= request.headers["content-type"].size()) {      // nothing after "boundary=" (also prevent segfault)
		return false;
	}
	for (size_t it = request.headers["content-type"].find('=') + 1; it < request.headers["content-type"].size(); it++) {
		if (request.headers["content-type"][it] != ' ')
			_boundaryValue.push_back(request.headers["content-type"][it]);
	}
	if (_boundaryValue.empty()) {       // boundary no value or the value is WS
		return false;
	}
	if (!_bodyBuf.size()) {     // no body
		return false;
	}
	if (request.headers.find("content-length") == request.headers.end() ||atoi(request.headers["content-length"].c_str()) > _bodyBuf.size()) {  // no content-length or content-length bigger than body
		return false;
	}
	if (!strstr(_bodyBuf.data(), _boundaryValue.c_str())) {     // no boundary value found in the body
		return false;
	}
    divideBody(_bodyBuf, _boundaryValue);
    if (!putIntoStruct()) {       // parse the body to find the file name and content
        return false;
    }
    for (size_t m = 0; m < _subBody.size(); m++)
    {
        if (_subBody[m].fileName.find("..") != std::string::npos) {     // not allow /../ appear in filename
            return false;
        }
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

void PostHandler::setPath(std::string root, std::string uri) {
    for (size_t i = 0; i < _subBody.size(); i++)
    {
        std::string tempPath = root + "/" + uri + "/" + _subBody[i].fileName;
        for (size_t i = 0; i < tempPath.size(); )
        {
            if (tempPath[i] == '/' && i + 1 < tempPath.size() && (tempPath[i + 1] == '/' || tempPath[i + 1] == '.')) {
                tempPath.erase(i + 1, 1);
            } else {
                i++;
            }
        }
        _subBody[i].filePath = tempPath;
    }
    
}

void PostHandler::handle(Connection* conn, HttpRequest& request, RouteConfig& config) {
    HttpResponse& resp = conn->_response;

    // check everything necessary first
    if (!_getValidation(conn, request, config)) {
        _setErrorResponse(resp, 400, "Bad Request");
        conn->setState(Connection::SendResponse);
        return;
    }

    setPath(config.root, request.uri);

    std::string uri = request.uri;
    _path = _normalizePath(config.root, uri);

    if (_path.empty()) {
        _setErrorResponse(resp, 403, "Forbidden");
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