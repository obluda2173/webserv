#include <HttpParser.h>

HttpParser::HttpParser() :
    _state(STATE_REQUEST_LINE),
    _bodyBytesRead(0),
    _expectedBodyLength(0) {}

bool HttpParser::feed(char* buffer, size_t length) {
    _buffer.append(buffer, length);
    parseBuffer();
    return _state == STATE_DONE;
}

void HttpParser::parseBuffer() {
    std::istringstream iss(_buffer);
    std::string line;

    if (_state == STATE_REQUEST_LINE) {
        if (!std::getline(iss, line)) {
            return;
        }
        std::istringstream lineStream(line);
        lineStream >> _currentRequest.method >> _currentRequest.uri >> _currentRequest.version;
        _state = STATE_HEADERS;
    }
    if (_state == STATE_HEADERS) {

        while (std::getline(iss, line) && line != "\r") {
            std::string key;
            std::string value;
            int length = 0;
            size_t separatorPos = line.find(':');
            key = line.substr(0, separatorPos);
            value = line.substr(separatorPos + 1, line.size() - separatorPos - 1);
            _currentRequest.headers.insert(std::make_pair(key, value));
        }
        
        
        //searching for "content_length"
        std::map<std::string, std::string>::iterator it;
        it = _currentRequest.headers.find("Content-Length");
        if (it != _currentRequest.headers.end()) {
            std::stringstream bodyLengthValue(it->second);
            bodyLengthValue >> _expectedBodyLength;
        }
        _state = STATE_BODY;
    }
    if (_state == STATE_BODY) {
        // std::string tempBody;
        // while (std::getline(iss, line))
        // {
        //     tempBody.append(line);
        // }
        // _currentRequest.body = tempBody.c_str();//body is not a const char*, so the error happened.

        // &_currentRequest.body = malloc((_expectedBodyLength + 1) * sizeof(char));
        
        /*
        pos = lein.find(\r\n\r\n)
        newbody = buffer.substr(pos)
        _currentRequest.body = newbody.c_str()
        */
        //todo
    }
}

HttpRequest HttpParser::getRequest() {
    if (_state != STATE_DONE) {
        throw std::runtime_error("Request not complete");
    }
    // Reset parser state for next request.
    HttpRequest completedRequest = _currentRequest;
    _state = STATE_REQUEST_LINE;
    _buffer.clear();
    _currentRequest = HttpRequest();
    _bodyBytesRead = 0;
    _expectedBodyLength = 0;
    return completedRequest;
}

int HttpParser::ready() {
    return _state == STATE_DONE;
}

int HttpParser::error(){return 0;}
int HttpParser::ready(){return 0;}
IHttpParser::~IHttpParser(){}
HttpParser::~HttpParser(){}