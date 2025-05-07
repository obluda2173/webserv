#include "Connection.h"
#include "HttpResponse.h"

Connection::Connection(sockaddr_storage addr, int fd, IHttpParser* prsr)
    : _state(ReadingHeaders), _addr(addr), _fd(fd), _prsr(prsr), _wrtr(NULL) {}

Connection::~Connection() {
    close(_fd);
    delete _prsr;
    delete _wrtr;
    delete _response.body;
}

void Connection::resetResponse() {
    delete _response.body;
    delete _wrtr;
    _wrtr = NULL;
    _response = HttpResponse{};
}

void Connection::readIntoBuf() {
    // Reserve space to minimize reallocations if receiving multiple chunks
    if (_buf.capacity() < _buf.size() + 1024)
        _buf.reserve(_buf.size() + 1024);

    // Get position to write at and increase size
    size_t oldSize = _buf.size();
    _buf.resize(oldSize + 1024);

    // Receive directly into string buffer
    ssize_t r = recv(_fd, &_buf[oldSize], 1024, 0);

    // Adjust size to actual bytes received
    if (r > 0)
        _buf.resize(oldSize + r);
    else
        _buf.resize(oldSize); // Restore original size if no data received
}

void Connection::sendResponse() {
    if (!_wrtr)
        _wrtr = new ResponseWriter(_response);

    char buffer[1024];
    size_t bytesWritten = _wrtr->write(buffer, 1024);
    size_t bytesSent = send(_fd, buffer, bytesWritten, 0);
    (void)bytesSent;
}

void Connection::parseBuf() {
    if (_prsr->error() || _prsr->ready()) {
        _prsr->resetPublic();
        _state = ReadingHeaders;
    }

    char* b = (char*)_buf.c_str();
    while (*b) {
        _prsr->feed(b, 1);
        if (_prsr->error() || _prsr->ready()) {
            _buf = b + 1;
            if (_prsr->error())
                _state = HandleBadRequest;
            else
                _state = Handling;
            return;
        }
        b++;
    }
    _buf = b;
}

int Connection::getFileDes() const { return _fd; }

Connection::STATE Connection::getState() const { return _state; }

struct sockaddr_storage Connection::getAddr() const { return _addr; }
