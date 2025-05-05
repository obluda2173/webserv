#include "Connection.h"

Connection::Connection(sockaddr_storage addr, int fd, IHttpParser* prsr)
    : _state(ReadingHeaders), _addr(addr), _fd(fd), _prsr(prsr), _wrtr(NULL) {}

Connection::~Connection() {
    close(_fd);
    delete _prsr;
    delete _wrtr;
    delete _response.body;
}

void Connection::readIntoBuf() {
    char newbuf[1024];
    ssize_t r = recv(_fd, newbuf, 1024, 0);
    newbuf[r] = '\0';
    _buf += newbuf;
}

void Connection::_send() {
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
