#include "ResponseWriter.h"
#include "utils.h"
#include <algorithm>

// Definition of the static member
const std::string ResponseWriter::CRLF = "\r\n";
const std::string ResponseWriter::WS = " ";

void ResponseWriter::_writeStatusLine() {
    if (_resp.version.empty()) {
        _headers += "HTTP/1.1" + WS;
    } else {
        _headers += _resp.version + WS;
    }
    _headers += to_string(_resp.statusCode) + WS;
    _headers += _resp.statusMessage;
    _headers += CRLF;
}

void ResponseWriter::_formatHeaders() {
    // status line
    _writeStatusLine();

    // headers
    if (!_resp.contentType.empty()) {
        _headers += "Content-Type: " + _resp.contentType + CRLF;
    }
    if (!_resp.contentLanguage.empty()) {
        _headers += "Content-Language: " + _resp.contentLanguage + CRLF;
    }
    for (std::map<std::string, std::string>::iterator it = _resp.headers.begin(); it != _resp.headers.end(); it++) {
        _headers += it->first + ": " + it->second + CRLF;
    }
    if (_resp.contentLength > 0)
        _headers += "Content-Length: " + to_string(_resp.contentLength) + CRLF;
    else
        _headers += "Content-Length: 0" + CRLF;

    _headers += CRLF;
}

size_t ResponseWriter::_writeHeaders(char* buffer, size_t maxSize) {
    size_t bytesToWrite = std::min(maxSize, _headers.size());
    memcpy(buffer, _headers.c_str(), bytesToWrite);
    return bytesToWrite;
}

size_t ResponseWriter::write(char* buffer, size_t maxSize) {
    size_t bytesWritten = 0;
    bool continueProcessing = true;
    while (continueProcessing) {
        switch (_state) {
        case FormatHeaders:
            _formatHeaders();
            _state = WritingHeaders;
            break;
        case WritingHeaders:
            bytesWritten = _writeHeaders(buffer, maxSize);
            maxSize -= bytesWritten;
            if (maxSize) {
                _state = WritingBody;
            } else {
                _headers = _headers.substr(bytesWritten, _headers.length());
                continueProcessing = false;
            }
            break;
        case WritingBody:
            if (_resp.body)
                bytesWritten += _resp.body->read(buffer + bytesWritten, maxSize);
            continueProcessing = false;
            break;
        }
    }
    return bytesWritten;
}
