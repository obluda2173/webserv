#include "ResponseWriter.h"
#include <string.h>

// Definition of the static member
const std::string ResponseWriter::CLRF = "\r\n";
const std::string ResponseWriter::WS = " ";

void ResponseWriter::_writeStatusLine() {
    _respString += _resp.version + WS;
    _respString += std::to_string(_resp.statusCode) + WS;
    _respString += _resp.statusMessage;
    _respString += CLRF;
}

void ResponseWriter::_writeHeaders() {
    _writeStatusLine();
    if (_resp.contentLength > 0)
        _respString += "Content-Length: " + std::to_string(_resp.contentLength) + CLRF;

    _respString += CLRF;
}

void ResponseWriter::_writeBody() {
    char buffer[1024];
    size_t readBytes = _resp.body->read(buffer, 1023);
    buffer[readBytes] = '\0';
    _respString += buffer;
}

int ResponseWriter::write(char* buffer, int maxSize) {
    (void)maxSize;
    _writeHeaders();

    if (_resp.body) {
        _writeBody();
    }

    memcpy(buffer, _respString.c_str(), _respString.length());
    return _respString.length();
}
