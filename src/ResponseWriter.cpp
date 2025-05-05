#include "ResponseWriter.h"
#include <algorithm>
#include <string.h>

// Definition of the static member
const std::string ResponseWriter::CRLF = "\r\n";
const std::string ResponseWriter::WS = " ";

void ResponseWriter::_writeStatusLine() {
    _headerString += _resp.version + WS;
    _headerString += std::to_string(_resp.statusCode) + WS;
    _headerString += _resp.statusMessage;
    _headerString += CRLF;
}

void ResponseWriter::_writeHeaders() {
    _writeStatusLine();
    if (_resp.contentLength > 0)
        _headerString += "Content-Length: " + std::to_string(_resp.contentLength) + CRLF;

    _headerString += CRLF;
}

size_t ResponseWriter::write(char* buffer, size_t maxSize) {
    if (!_headersWritten) {
        _writeHeaders();
        _headersWritten = true;
    }

    size_t bytesCopied = 0;
    if (!_headersCopied) {

        size_t bytesToCopy = std::min(maxSize, _headerString.size());
        memcpy(buffer, _headerString.c_str(), bytesToCopy); // TODO: maybe have it retry if it fails
        maxSize -= bytesToCopy;
        if (maxSize) {
            _headersCopied = true;
            bytesCopied = bytesToCopy;
        } else {
            _headerString = _headerString.substr(bytesToCopy, _headerString.length());
            return bytesToCopy;
        }
    }

    if (_resp.body) {
        size_t readBytes = _resp.body->read(buffer + bytesCopied, maxSize);
        return readBytes + bytesCopied;
    }
    return bytesCopied;
}
