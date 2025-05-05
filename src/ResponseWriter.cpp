#include "ResponseWriter.h"

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

void ResponseWriter::_writeBody() { _respString += _resp.body; }

std::string ResponseWriter::write() {
    _writeHeaders();
    _writeBody();
    return _respString;
}
