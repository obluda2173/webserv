#include "Connection.h"

Connection::~Connection() {
    close(_fd);
    delete _prsr;
}

Connection::Connection(sockaddr_storage addr, int fd, IHttpParser* prsr) : _addr(addr), _fd(fd), _prsr(prsr) {
    _state = ReadingHeaders;
}

Connection::STATE Connection::getState() const { return _state; }

void Connection::readIntoBuf() {
    char newbuf[1024];
    ssize_t r = recv(_fd, newbuf, 1024, 0);
    newbuf[r] = '\0';
    _buf += newbuf;
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
            if (_prsr->error()) {
                _state = WritingError;
            } else {
                _state = WritingResponse;
            }
            return;
        }
        b++;
    }
    _buf = b;
}

struct sockaddr_storage Connection::getAddr() const { return _addr; }
