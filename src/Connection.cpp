#include "Connection.h"
#include "HttpResponse.h"
#include <climits>
#include <cstring>
#include <string.h>

Connection::Connection(sockaddr_storage addr, int fd, IHttpParser* prsr, ISender* sender)
    : _state(ReadingHeaders), _addr(addr), _fd(fd), _prsr(prsr), _wrtr(NULL), _sender(sender) {
    _readBuf = Buffer();
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

void Connection::readIntoBuf() { _readBuf.recvNew(_fd); }

void Connection::parseBuf() {
    if (_prsr->error() || _prsr->ready()) {
        std::cout << "resetting in parseBuf" << std::endl;
        _prsr->resetPublic();
        _state = ReadingHeaders;
    }

    size_t count = 0;
    while (count < _readBuf.size()) {
        _prsr->feed(_readBuf.data() + count, 1);
        if (_prsr->error() || _prsr->ready()) {
            _readBuf.advance(count + 1);
            if (_prsr->ready()) {
                _request = _prsr->getRequest();
                _state = Handling;
            } else
                _state = HandleBadRequest;
            return;
        }
        count++;
    }
    _readBuf.clear();
}
void Connection::setReadBuf(std::string s) { _readBuf.assign(s); } // only used for testing

std::string Connection::getReadBuf() { return std::string(_readBuf.data(), _readBuf.size()); }

int Connection::getFileDes() const { return _fd; }

Connection::STATE Connection::getState() const { return _state; }

void Connection::setState(Connection::STATE state) { _state = state; };

struct sockaddr_storage Connection::getAddr() const { return _addr; }
