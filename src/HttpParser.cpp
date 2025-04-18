#include <HttpParser.h>

HttpParser::HttpParser() : _state(STATE_REQUEST_LINE) { _currentRequest.hasBody = false; }

HttpParser::~HttpParser() {}

void HttpParser::reset() {
    _state = STATE_REQUEST_LINE;
    _buffer.clear();
    _currentRequest = HttpRequest();
    _currentRequest.hasBody = false;
}

std::string toLower(const std::string &str);

void HttpParser::parseBuffer() {
    // TODO: impose a limit of 256 bytes for the header field (because of hashing, performance)
    // TODO: have a maximum length (_maxSizeHeader) of 4KB by default and maybe make it configurable (how does nginx do
    // it?)
    while (_state != STATE_DONE && _state != STATE_ERROR) {
        if (_buffer.size() > 8000) { // recommended limit TODO: refactor to a private data member (_)
            _state = STATE_ERROR;
            return;
        }

        size_t pos = _buffer.find("\r\n");
        if (pos == std::string::npos) {
            if (_buffer.find("\n") != std::string::npos) { // optional. but we consider invalid
                _state = STATE_ERROR;
            }
            return;
        }

        std::string line = _buffer.substr(0, pos);
        _buffer.erase(0, pos + 2);

        if (_state == STATE_REQUEST_LINE) {
            // TODO: factor out to a function or member function
            std::istringstream iss(line);
            std::string temp;
            iss >> _currentRequest.method >> _currentRequest.uri >> _currentRequest.version >> temp;
            if (_currentRequest.method.empty() || _currentRequest.uri.empty() || _currentRequest.version.empty() ||
                !temp.empty()) {
                _state = STATE_ERROR;
                return;
            }
            _state = STATE_HEADERS;
        } else if (_state == STATE_HEADERS) {
            if (line.empty()) {

                // TODO: factor out to a function or member function
                for (std::vector<std::pair<std::string, std::string>>::const_iterator it =
                         _currentRequest.headers.begin();
                     it != _currentRequest.headers.end(); ++it) {
                    if (it->first == "content-length" || it->first == "transfer-encoding") {
                        _currentRequest.hasBody = true;
                        break;
                    }
                }
                _state = STATE_DONE;
            } else {

                // TODO: factor out to a function or member function
                size_t sep = line.find(':');
                if (sep == std::string::npos) {
                    _state = STATE_ERROR;
                    return;
                }
                std::string key = toLower(line.substr(0, sep));
                std::string value = line.substr(sep + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                if (key.empty()) {
                    _state = STATE_ERROR;
                    return;
                }

                if (_currentRequest.headers.size() > 100) { // adjustable
                    _state = STATE_ERROR;
                    return;
                }

                _currentRequest.headers.push_back(std::make_pair(key, value));
            }
        }
    }
}

std::string toLower(const std::string &str) {
    std::string result = str;
    for (size_t i = 0; i < result.size(); ++i) {
        if (result[i] >= 'A' && result[i] <= 'Z') {
            result[i] = result[i] + ('a' - 'A'); // TODO: use cpp-way
        }
    }
    return result;
}

void HttpParser::feed(const char *buffer, size_t length) {
    _buffer.append(buffer, length);
    parseBuffer();
}

int HttpParser::ready() { return _state == STATE_DONE; }

int HttpParser::error() { return _state == STATE_ERROR; }

HttpRequest HttpParser::getRequest() {
    if (_state != STATE_DONE) {
        throw std::runtime_error("Request not complete");
    }
    HttpRequest result = _currentRequest;
    reset();
    return result;
}

// how we gonna solve this: we compare the bool hasBody
// to solve: if header is ending by multiple \r\n throw an error
