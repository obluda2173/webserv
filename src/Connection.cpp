#include "Connection.h"
#include "HttpResponse.h"
#include <climits>
#include <cstring>
#include <string.h>

Connection::Connection(sockaddr_storage addr, int fd, IHttpParser* prsr, ISender* sender)
    : _state(ReadingHeaders), _addr(addr), _fd(fd), _prsr(prsr), _wrtr(NULL), _sender(sender) {
    _readBuf = Buffer();
    _sendBuf = Buffer();
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

    _sendBuf.write(_wrtr);

    _sendBuf.send(_sender, _fd);

    if (_response.body && !_response.body->isDone())
        return;

    _state = Reset;
}

void Connection::readIntoBuf() { _readBuf.recv(_fd); }

void Connection::parseBuf() {
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
