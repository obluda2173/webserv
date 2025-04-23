#include <HttpParser.h>

HttpParser::HttpParser(Logger* logger)
    : _state(STATE_REQUEST_LINE), _totalProcessedSize(0), _maxHeaderKeySize(256), _maxHeaderSize(4096),
      _logger(logger) {}

HttpParser::HttpParser(size_t maxHeaderKeySize, size_t maxHeaderSize, Logger* logger)
    : _state(STATE_REQUEST_LINE), _totalProcessedSize(0), _maxHeaderKeySize(maxHeaderKeySize),
      _maxHeaderSize(maxHeaderSize), _logger(logger) {}

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
    if (_currentRequest.method.empty() || _currentRequest.uri.empty() || _currentRequest.version.empty() ||
        !tmp.empty()) {
        _state = STATE_ERROR;
    } else if (!_requestLineValidation(_currentRequest.method, _currentRequest.version)) {
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
    if (key.empty() || key.size() > _maxHeaderKeySize ||
        _currentRequest.headers.count(key) > 0) { // the _currentRequest.headers.count(key) > 0 check is to prevent
                                                  // duplicate headers, but we use map so this might have some issues
        _state = STATE_ERROR;
        return;
    }
    if (!_headerLineValidation(key, value)) {
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

bool checkValidMethod(const std::string& method) {
    return method == "GET" || method == "POST" || method == "PUT" || method == "DELETE" || method == "HEAD" ||
           method == "OPTIONS" || method == "PATCH";
}

bool checkValidVersion(const std::string& version) {
    double ver = 0;
    if (version.empty()) {
        return false;
    }
    if (version.find("HTTP/") != 0) {
        return false;
    }
    std::string verStr = version.substr(5);
    if (verStr.find('.') == std::string::npos && verStr.size() != 3) {
        return false;
    }
    if (!isdigit(verStr[0]) || verStr[1] != '.' || !isdigit(verStr[2])) {
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

bool HttpParser::_requestLineValidation(const std::string& method, const std::string& version) {
    if (!checkValidMethod(method)) {
        return false;
    }
    if (!checkValidVersion(version)) {
        return false;
    }
    return true;
}

bool checkCharsetBoundary(const std::string& str) {
    size_t pos = str.find("charset");
    if (pos != std::string::npos) {
        std::string charset = str.substr(pos + 7);
        size_t equal = charset.find('=');
        if (equal != std::string::npos) {
            charset = charset.substr(equal + 1);
            while (charset[0] == ' ') {
                charset.erase(0, 1);
            }
            if (charset.empty()) {
                return false;
            }
            for (size_t i = 0; i < charset.size(); i++) {
                if (!isalnum(charset[i]) && charset[i] != '-' && charset[i] != '_' && charset[i] != '.') {
                    return false;
                }
            }
        }
    } else {
        return false;
    }
    pos = str.find("boundary");
    if (pos != std::string::npos) {
        std::string boundary = str.substr(pos + 9);
        size_t equal = boundary.find('=');
        if (equal != std::string::npos) {
            boundary = boundary.substr(equal + 1);
            while (boundary[0] == ' ') {
                boundary.erase(0, 1);
            }
            if (boundary.empty()) {
                return false;
            }
            for (size_t i = 0; i < boundary.size(); i++) {
                if (!isalnum(boundary[i]) && boundary[i] != '-' && boundary[i] != '_' && boundary[i] != '.') {
                    return false;
                }
            }
        }
    } else {
        return false;
    }
    return true;
}

bool checkQValue(std::string str) {
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == 'q') {
            ++i;
            while (str[i] == ' ') {
                ++i;
            }
            if (str[i] == '=') {
                ++i;
                while (str[i] == ' ') {
                    ++i;
                }
                if (isdigit(str[i])) {
                    std::istringstream iss(str.substr(i, 3));
                    double q = 0;
                    iss >> q;
                    if (iss.fail() || !iss.eof()) {
                        return false;
                    }
                    if (q < 0 || q > 1) {
                        return false;
                    }
                    i += 3;
                } else {
                    return false;
                }
            }
        }
    }
    return true;
}

bool isValidHost(std::string str) {
    size_t colon = str.find(':');
    if (colon != std::string::npos) {
        std::string host = str.substr(0, colon);
        std::string port = str.substr(colon + 1);
        if (host.empty() || port.empty()) {
            return false;
        }
        for (size_t i = 0; i < port.size(); i++) {
            if (!isdigit(port[i])) {
                return false;
            }
        }
    } else {
        if (str.empty()) {
            return false;
        }
    }
    return true;
}

bool isValidContentType(const std::string& str) {
    size_t slash = str.find('/');
    if (slash == std::string::npos || slash == 0 ||
        slash == str.size() - 1) // the char '/' must be in the middle of the string
        return false;
    std::string type = str.substr(0, slash);
    if (type != "application" && type != "text" && type != "image" && type != "audio" && type != "video" &&
        type != "multipart" && type != "font" && type != "model" && type != "message" && type != "example") {
        return false;
    }
    if (checkCharsetBoundary(str) == false) {
        return false;
    }
    return true;
}

bool isValidAccept(const std::string& str) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, ',')) {
        tokens.push_back(token);
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        std::string token2 = tokens[i];
        size_t slash = token2.find('/');
        if (slash == std::string::npos || slash == 0 ||
            slash == token2.size() - 1) // the char '/' must be in the middle of the string
            return false;
        std::string type = token2.substr(0, slash);
        if (type != "application" && type != "text" && type != "image" && type != "audio" && type != "video" &&
            type != "multipart" && type != "font" && type != "model" && type != "message" && type != "example") {
            return false;
        }
        if (checkCharsetBoundary(token2) == false) {
            return false;
        }
        if (checkQValue(token2) == false) {
            return false;
        }
    }
    return true;
}

bool isValidAcceptEncoding(const std::string& str) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, ',')) {
        tokens.push_back(token);
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        std::string token2 = tokens[i];
        if (token2 != "gzip" && token2 != "deflate" && token2 != "br" && token2 != "compress" && token2 != "identity") {
            return false;
        }
        if (checkQValue(token2) == false) {
            return false;
        }
    }
    return true;
}

bool isValidAcceptLanguage(const std::string& str) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, ',')) {
        tokens.push_back(token);
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        std::string token2 = tokens[i];
        size_t dash = token2.find('-');
        if (dash != std::string::npos &&
            (dash == 0 || dash == token2.size() - 1)) // the char '-' should be in the middle of the string(if appears)
            return false;
        std::string lang = token2.substr(0, dash);
        if (!lang.empty() && lang.length() != 2) {
            return false;
        }
        if (dash != std::string::npos) {
            std::string region = token2.substr(dash + 1);
            if (region.empty()) {
                return false; // A region code must follow the '-'
            }
        }
        if (checkQValue(token2) == false) {
            return false;
        }
    }
    return true;
}

bool isValidCookie(std::string str) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, ';')) {
        tokens.push_back(token);
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        std::string token2 = tokens[i];
        size_t equal = token2.find('=');
        if (equal == std::string::npos || equal == 0 ||
            equal == token2.size() - 1) // the char '=' must be in the middle of the string
            return false;
        std::string key = token2.substr(0, equal);
        if (key.empty()) {
            return false;
        }
        if (checkQValue(token2) == false) {
            return false;
        }
    }
    return true;
}

bool isValidRange(const std::string& str) {
    size_t pos = str.find("bytes=");
    if (pos == std::string::npos) {
        return false;
    }
    std::string range = str.substr(pos + 6);
    std::vector<std::string> tokens;
    std::istringstream iss(range);
    std::string token;
    while (std::getline(iss, token, ',')) {
        tokens.push_back(token);
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        std::string token2 = tokens[i];
        size_t dash = token2.find('-');
        if (dash == std::string::npos || dash == 0 ||
            dash == token2.size() - 1) // the char '-' must be in the middle of the string
            return false;
        std::string start = token2.substr(0, dash);
        std::string end = token2.substr(dash + 1);
        if (start.empty() || end.empty()) {
            return false;
        }
        for (size_t j = 0; j < start.size(); j++) {
            if (!isdigit(start[j])) {
                return false;
            }
        }
        for (size_t k = 0; k < end.size(); k++) {
            if (!isdigit(end[k])) {
                return false;
            }
        }
        if (checkQValue(token2) == false) {
            return false;
        }
    }
    return true;
}

bool specificHeaderValidation(const std::string& key, const std::string& value,
                              Logger* _logger) { // so far, I added these functions, but if we need more, I can add them
    if (key == "Host") {
        if (!isValidHost(value)) {
            _logger->log("ERROR", "specificHeaderValidation: Invalid Host header");
            return false;
        }
    }
    if (key == "Content-Length") {
        std::istringstream iss(value);
        int contentLength;
        iss >> contentLength;
        if (contentLength < 0) {
            _logger->log("ERROR", "specificHeaderValidation: Invalid Content-Length header");
            return false;
        }
    }
    if (key == "Transfer-Encoding") {
        const std::string chunked = "chunked";
        if (value.length() < chunked.length() || value.substr(value.length() - chunked.length()) != chunked) {
            _logger->log("ERROR", "specificHeaderValidation: Invalid Transfer-Encoding header");
            return false;
        }
    }
    if (key == "Connection") {
        if (value != "keep-alive" && value != "close") {
            _logger->log("ERROR", "specificHeaderValidation: Invalid Connection header");
            return false;
        }
    }
    if (key == "Content-Type") {
        if (isValidContentType(value) == false) {
            _logger->log("ERROR", "specificHeaderValidation: Invalid Content-Type header");
            return false;
        }
    }
    if (key == "Accept") {
        if (isValidAccept(value) == false) {
            _logger->log("ERROR", "specificHeaderValidation: Invalid Accept header");
            return false;
        }
    }
    if (key == "Accept-Encoding") {
        if (isValidAcceptEncoding(value) == false) {
            _logger->log("ERROR", "specificHeaderValidation: Invalid Accept-Encoding header");
            return false;
        }
    }
    if (key == "Accept-Language") {
        if (isValidAcceptLanguage(value) == false) {
            _logger->log("ERROR", "specificHeaderValidation: Invalid Accept-Language header");
            return false;
        }
    }
    if (key == "Cookie") {
        if (isValidCookie(value) == false) {
            _logger->log("ERROR", "specificHeaderValidation: Invalid Cookie header");
            return false;
        }
    }
    if (key == "Range") {
        if (isValidRange(value) == false) {
            _logger->log("ERROR", "specificHeaderValidation: Invalid Range header");
            return false;
        }
    }
    return true;
}

bool HttpParser::_headerLineValidation(const std::string& key, const std::string& value) {
    if (key.empty() || value.empty()) {
        _logger->log("ERROR", "_headerLineValidation: Header key or value is empty");
        return false;
    }
    if (key.find(' ') != std::string::npos && key.find('\t') != std::string::npos &&
        key.find('\r') != std::string::npos && key.find('\n') != std::string::npos) {
        _logger->log("ERROR", "_headerLineValidation: Header key contains invalid characters");
        return false;
    }
    // this part is commented out because it is not so stayble
    // for (size_t i = 1; i < value.size(); i++) {
    // 	if ((value[i] == ' ' || value[i] == '\t') && (value[i - 1] != ',' && value[i - 1] != ';')) {
    // 		_logger->log("ERROR", "_headerLineValidation: Header value contains invalid characters");
    // 		return false;
    // 	}
    // }
    if (key == "content-length") {
        if (_currentRequest.headers.count("transfer-encoding") > 0) {
            _logger->log("ERROR",
                         "_headerLineValidation: Content-Length and Transfer-Encoding cannot be used together");
            return false;
        }
    }
    if (key == "transfer-encoding") {
        if (_currentRequest.headers.count("content-length") > 0) {
            _logger->log("ERROR",
                         "_headerLineValidation: Transfer-Encoding and Content-Length cannot be used together");
            return false;
        }
    }
    if (specificHeaderValidation(key, value, _logger) == false) {
        return false;
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
