#include "Connection.h"
#include "HttpResponse.h"
#include <climits>
#include <cstring>
#include <string.h>

Connection::Connection(sockaddr_storage addr, int fd, IHttpParser* prsr, size_t readSize, ISender* sender)
    : _state(ReadingHeaders), _addr(addr), _fd(fd), _prsr(prsr), _wrtr(NULL), _readSize(readSize), _sender(sender) {
    _sendBuf = std::vector<char>(1024);
    _sendBufUsedSize = 0;
}

Connection::~Connection() {
    close(_fd);
    delete _prsr;
    delete _wrtr;
    delete _sender;
    delete _response.body;
}

HttpRequest Connection::getRequest() { return _request; }

void Connection::resetResponse() {
    delete _response.body;
    delete _wrtr;
    _wrtr = NULL;
    _response = HttpResponse{};
}

void Connection::readIntoBuf() {
    // Reserve space to minimize reallocations if receiving multiple chunks
    if (_buf.capacity() < _buf.size() + _readSize)
        _buf.reserve(_buf.size() + _readSize);

    // Get position to write at and increase size
    size_t oldSize = _buf.size();
    _buf.resize(oldSize + _readSize);

    // Receive directly into string buffer
    ssize_t r = recv(_fd, &_buf[oldSize], _readSize, 0);
    // Adjust size to actual bytes received
    if (r > 0)
        _buf.resize(oldSize + r);
    else
        _buf.resize(oldSize); // Restore original size if no data received
}

void Connection::sendResponse() {
    if (!_wrtr)
        _wrtr = new ResponseWriter(_response);

    // Writing new data
    size_t bytesWritten = _wrtr->write(_sendBuf.data() + _sendBufUsedSize, _sendBuf.size() - _sendBufUsedSize);
    _sendBufUsedSize += bytesWritten;

    // Sending data
    size_t bytesSent = _sender->_send(_fd, _sendBuf.data(), _sendBufUsedSize);

    // Update after sending
    if (bytesSent > 0) {
        memmove(_sendBuf.data(), _sendBuf.data() + bytesSent, _sendBufUsedSize - bytesSent);
        _sendBufUsedSize -= bytesSent;
    }

    if (_response.body == NULL)
        return;
    if (_response.body->isDone())
        _state = Reset;
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
            if (_prsr->ready()) {
                _request = _prsr->getRequest();
                _state = Handling;
            } else
                _state = HandleBadRequest;
            return;
        }
        b++;
    }
    _buf = b;
}

int Connection::getFileDes() const { return _fd; }

Connection::STATE Connection::getState() const { return _state; }

struct sockaddr_storage Connection::getAddr() const { return _addr; }
