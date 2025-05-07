#include "PostHandler.h"
std::map<std::string, std::string> PostHandler::mimeTypes = std::map<std::string, std::string>();

bool PostHandler::_divideBody(const std::vector<char>& bodyBuf, const std::string& boundary) {
    std::string dashBoundary = "--" + boundary;
    std::string endBoundary = dashBoundary + "--";

    std::vector<char> boundaryVec(dashBoundary.begin(), dashBoundary.end());
    std::vector<char> endBoundaryVec(endBoundary.begin(), endBoundary.end());

    std::vector<char>::const_iterator pos = bodyBuf.begin();
    std::vector<char>::const_iterator bodyEnd = bodyBuf.end();

    int count = 0;

    while (true) {
        if (count > 50) {       // here to set the MAX number of the subBody
            return false;
        }

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
        count++;
    }
    return true;
}

bool PostHandler::_putIntoStruct(void) {
    // now put everything into the vector struct
    for (size_t i = 0; i < _parts.size(); i++)
    {
        inBoundary tempSubBody;
        std::string token = "filename=";
        std::vector<char> tokenVec(token.begin(), token.end());
        std::vector<char>::const_iterator tokenPos = std::search(_parts[i].begin(), _parts[i].end(), tokenVec.begin(), tokenVec.end());
        if (tokenPos == _parts[i].end()) {      // no "filename=" appear
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
        if (tokenPos == _parts[i].end()) {      // no "\r\n\r\n" appear
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

bool PostHandler::_postValidation(Connection* conn, HttpRequest& request, RouteConfig& config) {
    if (request.uri.empty() || config.root.empty()) {       // uri and root should not empty
        return false;
    }
    if (request.uri.find("..") != std::string::npos) {      // not allow /../ appear
        return false;
    }
    if (_bodyBuf.size())
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
    if (!_divideBody(_bodyBuf, _boundaryValue)) {       // too much subBody
        return false;
    }
    if (!_putIntoStruct()) {       // parse the body to find the file name and content
        return false;
    }
    for (size_t m = 0; m < _subBody.size(); m++)
    {
        if (_subBody[m].fileName.empty()) {     // filename value should not empty
            _subBody[m].pathValid = false;
        }
        if (_subBody[m].fileName.find("..") != std::string::npos) {     // not allow /../ appear in filename
            return false;
        }
        _subBody[m].fileType = _getMimeType(_subBody[m].fileName);
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

void PostHandler::_setPath(std::string root, std::string uri) {     // make the path to the DIR without "//" or "/./"
    const size_t MAX_PATH_LENGTH = 4096;
    for (size_t i = 0; i < _subBody.size(); i++)
    {
        if (_subBody[i].pathValid == false) {       // if path got some error, skip
            continue;
        }
        if (root.empty() || uri.empty()) {
            _subBody[i].pathValid = false;
        }
        std::string tempPath = root + "/" + uri + "/";
        for (size_t i = 0; i < tempPath.size(); )
        {
            if (tempPath[i] == '/' && i + 1 < tempPath.size() && (tempPath[i + 1] == '/' || tempPath[i + 1] == '.')) {
                tempPath.erase(i + 1, 1);
            } else {
                i++;
            }
        }
        if (tempPath.size() > MAX_PATH_LENGTH) {        // the path should not too long
            _subBody[i].pathValid = false;
        }
        _subBody[i].noFileNamePath = tempPath;
    }
}

void PostHandler::_setPathWithFileName(void) {      // make the path to the FILE without "//" or "/./"
    const size_t MAX_PATH_LENGTH = 4096;
    for (size_t i = 0; i < _subBody.size(); i++)
    {
        if (_subBody[i].pathValid == false) {       // if path got some error, skip
            continue;
        }
        std::string tempPath = _subBody[i].noFileNamePath + "/" + _subBody[i].fileName;
        for (size_t i = 0; i < tempPath.size(); )
        {
            if (tempPath[i] == '/' && i + 1 < tempPath.size() && (tempPath[i + 1] == '/' || tempPath[i + 1] == '.')) {
                tempPath.erase(i + 1, 1);
            } else {
                i++;
            }
        }
        if (tempPath.size() > MAX_PATH_LENGTH) {        // the path should not too long
            _subBody[i].pathValid = false;
        }
        _subBody[i].filePath = tempPath;
    }
}

void PostHandler::_pathValidator(void) {
    for (size_t i = 0; i < _subBody.size(); i++) {
        if (_subBody[i].pathValid == false) {       // if path got some error, skip
            continue;
        }
        if (stat(_subBody[i].noFileNamePath.c_str(), &_pathStat) == 0) {        // the file/dir which path point to, is exist
            _subBody[i].pathExist = true;
        }
        if (access(_path.c_str(), R_OK) == 0) {         // the path is accessable
            _subBody[i].pathAccessable = true;
        }
        if (_subBody.size() > 1 && S_ISREG(_pathStat.st_mode)) {       // if there are multiple upload, path (root + uri) should not point to a file
            _subBody[i].pathValid = false;
        }
        if (_subBody.size() == 1 && S_ISREG(_pathStat.st_mode)) {       // if there is one upload and path (root + uri) point to a file, replace the file
            _subBody[i].replaceFile = true;
        }
        if (!S_ISREG(_pathStat.st_mode) && !S_ISDIR(_pathStat.st_mode)) {
            _subBody[i].pathValid = false;
        }
    }
}

void PostHandler::_pathWithFileNameValidator(void) {
    for (size_t i = 0; i < _subBody.size(); i++) {
        if (_subBody[i].pathValid == false || _subBody[i].pathExist == false || _subBody[i].pathAccessable == false) {       // if path got some error, skip
            continue;
        }
        if (stat(_subBody[i].noFileNamePath.c_str(), &_pathStat) == 0) {
            _subBody[i].fileExist = true;       // this means there is a file/dir
            if (S_ISREG(_pathStat.st_mode)) {
                _subBody[i].fileValid = true;       // this means this is a file now, not a dir
            }
            if (access(_subBody[i].filePath.c_str(), W_OK) == 0) {      //the file is accessable
                _subBody[i].fileAccessalbe = true;
            }
        }
    }
}

void PostHandler::replaceFile(int index) {
    if (remove(_subBody[index].noFileNamePath.c_str())) {
        _subBody[index].uploadFailure = true;
    }
    
    std::string tempPath = _subBody[index].noFileNamePath;
    int pos = tempPath.rfind('/');
    tempPath.erase(tempPath.begin() + pos, tempPath.end());
    tempPath += _subBody[index].fileName;
    std::ofstream fileToCreat(tempPath);
    if (!fileToCreat) {
        _subBody[index].uploadFailure = true;
    }
    for (size_t m = 0; m < _subBody[index].bodyContent.size(); m++)
    {
        if (m < _subBody[index].bodyContent.size() - 1 && _subBody[index].bodyContent[m] == '\r' && _subBody[index].bodyContent[m + 1] == '\n') {
            continue;
        }
        fileToCreat << _subBody[index].bodyContent[m];
    }
    fileToCreat.close();
}

void PostHandler::_writeIntoFile() {
    for (size_t i = 0; i < _subBody.size(); i++)
    {
        if (_subBody[i].fileExist == false || _subBody[i].fileValid == false || _subBody[i].fileAccessalbe == false) {      //if the file got some error, skip
            if (_subBody[i].replaceFile == true && _subBody[i].pathExist == true) {      // we need to replace the file
                replaceFile(i);
            } else if (_subBody[i].pathValid == true && _subBody[i].pathExist == true && _subBody[i].pathAccessable == true
                && _subBody[i].fileExist == false && _subBody[i].fileValid == false && _subBody[i].fileAccessalbe == false) {       // we need to create a file
                    std::ofstream createFile(_subBody[i].filePath);
                    for (size_t m = 0; m < _subBody[i].bodyContent.size(); m++)
                    {
                        if (m < _subBody[i].bodyContent.size() - 1 && _subBody[i].bodyContent[m] == '\r' && _subBody[i].bodyContent[m + 1] == '\n') {
                            continue;
                        }
                        createFile << _subBody[i].bodyContent[m];
                    }
            } else {        // something wrong with the file or with the accessability
                _subBody[i].uploadFailure = true;
                continue;
            }
        } else {        // everything is good
            std::ofstream appendFile(_subBody[i].filePath, std::ios::app);      // append the body
            if (!appendFile) {
                _subBody[i].uploadFailure = true;
                continue;
            }
            for (size_t m = 0; m < _subBody[i].bodyContent.size(); m++)
            {
                if (m < _subBody[i].bodyContent.size() - 1 && _subBody[i].bodyContent[m] == '\r' && _subBody[i].bodyContent[m + 1] == '\n') {
                    continue;
                }
                appendFile << _subBody[i].bodyContent[m];
            }
        }
    }
}

void PostHandler::handle(Connection* conn, HttpRequest& request, RouteConfig& config) {
    HttpResponse& resp = conn->_response;

    // check everything necessary first
    if (!_postValidation(conn, request, config)) {
        _setErrorResponse(resp, 400, "Bad Request");
        conn->setState(Connection::SendResponse);
        return;
    }

    _setPath(config.root, request.uri);     // also check the root and uri here

    _setPathWithFileName();     // also check file name here

    _pathValidator();       // here check the path (root + uri) is exist, accessable, valid or need to replace the file

    _pathWithFileNameValidator();       // here check the file path (root + uri + filename) is exist, accessable or valid

    _writeIntoFile();       // here write into the file, replace the file or create a file

    

    std::string uri = request.uri;

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