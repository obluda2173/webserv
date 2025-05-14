#include "Connection.h"
#include "HttpResponse.h"
#include <climits>
#include <cstring>
#include <string.h>

Connection::Connection(sockaddr_storage addr, int fd, IHttpParser* prsr, ISender* sender)
    : _state(ReadingHeaders), _addr(addr), _fd(fd), _prsr(prsr), _wrtr(NULL), _sender(sender) {
    _readBuf = std::vector< char >(1024);
    _readBufUsedSize = 0;
    _sendBuf = std::vector< char >(1024);
    _sendBufUsedSize = 0;
}

Connection::~Connection() {
    close(_fd);
    delete _prsr;
    delete _wrtr;
    delete _sender;
    delete _response.body;
    delete uploadCtx.file;
}

HttpRequest Connection::getRequest() { return _request; }

void Connection::resetResponse() {
    delete _response.body;
    delete _wrtr;
    _wrtr = NULL;
    _response = HttpResponse();
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

    if (_response.body && !_response.body->isDone())
        return;

    _state = Reset;
}

void Connection::readIntoBuf() {
    // Receive directly into string buffer
    ssize_t r = recv(_fd, _readBuf.data() + _readBufUsedSize, _readBuf.size() - _readBufUsedSize, 0);
    _readBufUsedSize += r;
}

void Connection::parseBuf() {
    if (_prsr->error() || _prsr->ready()) {
        std::cout << "resetting in parseBuf" << std::endl;
        _prsr->resetPublic();
        _state = ReadingHeaders;
    }

    char* b = _readBuf.data();
    size_t count = 0;
    while (count < _readBufUsedSize) {
        _prsr->feed(b, 1);
        if (_prsr->error() || _prsr->ready()) {
            memmove(_readBuf.data(), b + 1, _readBufUsedSize - (count + 1));
            _readBufUsedSize -= (count + 1);
            if (_prsr->ready()) {
                _request = _prsr->getRequest();
                _state = Handling;
            } else
                _state = HandleBadRequest;
            return;
        }
        b++;
        count++;
    }
    _readBufUsedSize = 0;
}
void Connection::setReadBuf(std::string s) {
    if (s.length() > _readBuf.size()) {
        std::cout << "string is bigger than readBuf" << std::endl;
        _exit(1);
    }
    _readBuf.assign(s.begin(), s.end());
    _readBufUsedSize = s.length();
}

int Connection::getFileDes() const { return _fd; }

Connection::STATE Connection::getState() const { return _state; }

void Connection::setState(Connection::STATE state) { _state = state; };

std::string Connection::getReadBuf() { return std::string(_readBuf.data(), _readBufUsedSize); }

struct sockaddr_storage Connection::getAddr() const { return _addr; }
