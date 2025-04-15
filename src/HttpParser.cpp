#include <HttpParser.h>

HttpParser::HttpParser() :
    _state(STATE_REQUEST_LINE),
    _expectedBodyLength(0),
    _isChunked(false), 
    _chunkSize(0), 
    _chunkState(0) {}

bool HttpParser::feed(const char* buffer, size_t length) {
    _buffer.append(buffer, length);
    parseBuffer();
    return _state == STATE_DONE;
}

void HttpParser::parseBuffer() {
    while (_state != STATE_DONE && _state != STATE_ERROR) {
        if (_state == STATE_REQUEST_LINE || _state == STATE_HEADERS) {
            size_t pos = _buffer.find("\r\n");
            if (pos == std::string::npos)
                return; // Wait for more data

            std::string line = _buffer.substr(0, pos);
            _buffer.erase(0, pos + 2);

            if (_state == STATE_REQUEST_LINE) {
                std::istringstream iss(line);
                iss >> _currentRequest.method >> _currentRequest.uri >> _currentRequest.version;
                _state = STATE_HEADERS;
            } else if (_state == STATE_HEADERS) {
                if (line.empty()) {
                    std::map<std::string, std::string>::iterator it;
                    it = _currentRequest.headers.find("Transfer-Encoding");
                    if (it != _currentRequest.headers.end() && it->second == "chunked") {
                        _isChunked = true;
                        _state = STATE_CHUNKED_BODY;
                    } else {
                        it = _currentRequest.headers.find("Content-Length");
                        if (it != _currentRequest.headers.end()) {
                            std::stringstream ss(it->second);
                            ss >> _expectedBodyLength;
                        } else {
                            _expectedBodyLength = 0;
                        }
                        _state = STATE_BODY;
                    }
                } else {
                    size_t sep = line.find(':');
                    if (sep != std::string::npos) {
                        std::string key = line.substr(0, sep);
                        std::string value = line.substr(sep + 1);
                        value.erase(0, value.find_first_not_of(" \t")); // Trim leading whitespace
                        _currentRequest.headers[key] = value;
                    }
                }
            }
        } else if (_state == STATE_BODY) {
            if (_expectedBodyLength == 0) {
                _state = STATE_DONE;
            } else if (_buffer.size() >= _expectedBodyLength) {
                _currentRequest.body = (_buffer.substr(0, _expectedBodyLength));
                _buffer.erase(0, _expectedBodyLength);
                _state = STATE_DONE;
            } else {
                return; // Wait for more data
            }
        } else if (_state == STATE_CHUNKED_BODY) {
            while (true) {
                if (_chunkState == 0) { // Reading chunk size
                    size_t pos = _buffer.find("\r\n");
                    if (pos == std::string::npos)
                        return; // Wait for more data

                    std::string sizeLine = _buffer.substr(0, pos);
                    _buffer.erase(0, pos + 2);
                    
                    std::stringstream ss(sizeLine);
                    ss >> std::hex >> _chunkSize; //the number we received is hexadecimal, so we need to convert it
                    if (ss.fail() || !ss.eof()) {
                        _state = STATE_ERROR;
                        return;
                    }
                    
                    if (_chunkSize == 0) {
                        _buffer.erase(0, 2);
                        _state = STATE_DONE;
                        break;
                    }
                    
                    _chunkState = 1; // Switch to reading chunk data
                } else if (_chunkState == 1) { // Reading chunk data
                    if (_buffer.size() >= _chunkSize + 2) {
                        _currentRequest.body.append(_buffer.substr(0, _chunkSize));
                        _buffer.erase(0, _chunkSize);
                        if (_buffer.size() >= 2 && _buffer.substr(0, 2) == "\r\n") {
                            _buffer.erase(0, 2);
                            _chunkState = 0; // Next chunk size
                        } else {
                            _state = STATE_ERROR; // Malformed chunk
                            break;
                        }
                    } else {
                        return; // Wait for more data
                    }
                }
            }
        }
    }
}

HttpRequest HttpParser::getRequest() {
    if (_state != STATE_DONE) {
        throw std::runtime_error("Request not complete");
    }
    HttpRequest result = _currentRequest;
    reset();
    return result;
}

void HttpParser::reset() {
    _state = STATE_REQUEST_LINE;
    _buffer.clear();
    _currentRequest = HttpRequest();
    _expectedBodyLength = 0;
    _isChunked = false;
    _chunkSize = 0;
    _chunkState = 0;
}

int HttpParser::ready() {
    return _state == STATE_DONE;
}

int HttpParser::error() {
    return _state == STATE_ERROR;
}