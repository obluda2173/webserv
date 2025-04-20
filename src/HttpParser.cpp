#include <HttpParser.h>

HttpParser::HttpParser() : 
    _state(STATE_REQUEST_LINE),
    _totalProcessedSize(0),
    _maxHeaderKeySize(256),
    _maxHeaderSize(4096){}

HttpParser::HttpParser(size_t maxHeaderKeySize, size_t maxHeaderSize) :
    _state(STATE_REQUEST_LINE),
    _totalProcessedSize(0),
    _maxHeaderKeySize(maxHeaderKeySize),
    _maxHeaderSize(maxHeaderSize) {}

HttpParser::~HttpParser() {}

std::string toLower(const std::string& str) {
    std::string result = str;
    for (char* ptr = &result[0]; ptr < &result[0] + result.size(); ++ptr) {
        *ptr = static_cast<char>(tolower(*ptr));
    }
    return result;
}

bool HttpParser::_extractNextLine(std::string& line) {
    size_t pos = _buffer.find("\r\n");
    if (pos != std::string::npos) {
        size_t lineSize = pos + 2;
        if (_totalProcessedSize + lineSize > _maxHeaderSize) {
            _state = STATE_ERROR;
            return false;
        }
        line = _buffer.substr(0, pos);
        _buffer.erase(0, pos + 2);
        _totalProcessedSize += lineSize;
        return true;
    }
    return false;
}

void HttpParser::_parseRequestLine(const std::string& line) {
    std::istringstream iss(line);
    std::string tmp;
    iss >> _currentRequest.method >> _currentRequest.uri >> _currentRequest.version >> tmp;
    if (_currentRequest.method.empty() || _currentRequest.uri.empty() ||
        _currentRequest.version.empty() || !tmp.empty()) {
        _state = STATE_ERROR;
    } else if (!_requestLineValidation(_currentRequest.method , _currentRequest.uri, _currentRequest.version)) {
        _state = STATE_ERROR;
    } else {
        _state = STATE_HEADERS;
    }
}

void HttpParser::_parseHeader(const std::string& line) {
    size_t sep = line.find(':');
    if (sep == std::string::npos) {
        _state = STATE_ERROR;
        return;
    }
    std::string key = toLower(line.substr(0, sep));
    std::string value = line.substr(sep + 1);
    value.erase(0, value.find_first_not_of(" \t"));
    if (key.empty() || key.size() > _maxHeaderKeySize || _currentRequest.headers.count(key) > 0) {//the _currentRequest.headers.count(key) > 0 check is to prevent duplicate headers, but we use map so this might have some issues
        _state = STATE_ERROR;
        return;
    }
    if (!_headerLineValidation(key , value)) {
        _state = STATE_ERROR;
        return;
    }
    _currentRequest.headers[key] = value;
}

void HttpParser::_handlePartialLine() {
    if (_buffer.find("\n") != std::string::npos) {
        _state = STATE_ERROR;
    } else if (_totalProcessedSize + _buffer.size() > _maxHeaderSize) {
        _state = STATE_ERROR;
    }
}

void HttpParser::_reset() {
    _state = STATE_REQUEST_LINE;
    _buffer.clear();
    _currentRequest = HttpRequest();
}

void HttpParser::_parseBuffer() {
    while (_state != STATE_DONE && _state != STATE_ERROR) {
        std::string line;
        if (_extractNextLine(line)) {
            if (_state == STATE_REQUEST_LINE) {
                _parseRequestLine(line);
            } else if (_state == STATE_HEADERS) {
                if (!line.empty()) {
                    _parseHeader(line);
                } else {
                    _state = STATE_DONE;
                }
            }
        } else {
            _handlePartialLine();
            return;
        }
    }
}

bool	checkValidMethod(const std::string& method) {
	return method == "GET" || method == "POST" || method == "PUT" || method == "DELETE" ||
			method == "HEAD" || method == "OPTIONS" || method == "PATCH";
}

bool	checkValidVersion(const std::string& version) {
	double ver = 0;
	if (version.empty()) {
		return false;
	}
	if (version.find("HTTP/") != 0) {
		return false;
	}
	std::string verStr = version.substr(5);
	if (verStr.find('.') == std::string::npos) {
		return false;
	}
	std::istringstream iss(verStr);
	iss >> ver;
	if (iss.fail() || !iss.eof()) {
		return false;
	}
	if (ver < 0 || ver > 2) {
		return false;
	}
	return true;
}

bool	checkValidURI(const std::string& uri) {
	return uri.find("http://") == 0 || uri.find("https://") == 0 || 
			uri.find("ftp://") == 0 || uri.find("file://") == 0;
}

bool	HttpParser::_requestLineValidation(const std::string& method, const std::string& uri, const std::string& version) {
	if (!checkValidMethod(method)) {
		return false;
	}
	if (!checkValidURI(uri)) {
		return false;
	}
	if (!checkValidVersion(version)) {
		return false;
	}
	return true;
}

bool HttpParser::_headerLineValidation(const std::string& key, const std::string& value) {
	if (key.empty() || value.empty()) {
		return false;
	}
	if (key.find(' ') != std::string::npos && key.find('\t') != std::string::npos && key.find('\r') != std::string::npos && key.find('\n') != std::string::npos) {
		return false;
	}
	for (size_t i = 0; i < value.size(); i++)
	{
		if ((value[i] == ' ' || value[i] == '\t') && (value[i - 1] != ',' || value[i - 1] != ';')) {
			return false;
		}
	}
	
    return true;
}

void HttpParser::feed(const char* buffer, size_t length) {
    _buffer.append(buffer, length);
    _parseBuffer();
}

int HttpParser::ready() { return _state == STATE_DONE; }

int HttpParser::error() { return _state == STATE_ERROR; }

HttpRequest HttpParser::getRequest() {
    if (_state != STATE_DONE) {
        throw std::runtime_error("Request not complete");
    }
    HttpRequest result = _currentRequest;
    _reset();
    return result;
}
