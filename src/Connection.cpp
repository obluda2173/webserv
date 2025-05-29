#include "Connection.h"
#include "HttpResponse.h"
#include "Router.h"
#include "handlerUtils.h"
#include "httpParsing.h"
#include <climits>
#include <cstring>
#include <signal.h>
#include <string.h>
#include <sys/types.h>

Connection::Connection(sockaddr_storage addr, int fd, std::string addrPort, IHttpParser* prsr, ISender* sender)
    : _state(ReadingHeaders), _addr(addr), _fd(fd), _addrPort(addrPort), _prsr(prsr), _wrtr(NULL), _sender(sender) {
    _readBuf = Buffer();
    _sendBuf = Buffer();
    _hdlr = NULL;
    _bodyFinished = false;
    cgiCtx.cgiPipeStdin = -1;
    cgiCtx.cgiPipeStdout = -1;
    cgiCtx.cgiPid = -1;
}

Connection::~Connection() {
    close(_fd);
    delete _prsr;
    if (cgiCtx.cgiPipeStdout != -1)
        close(cgiCtx.cgiPipeStdout);
    if (cgiCtx.cgiPipeStdin != -1)
        close(cgiCtx.cgiPipeStdin);
    if (cgiCtx.cgiPid != -1)
        kill(cgiCtx.cgiPid, SIGTERM);

    delete _wrtr;
    delete _sender;
    delete _response.body;
    if (uploadCtx.file)
        delete uploadCtx.file;
}

HttpRequest Connection::getRequest() { return _request; }

void Connection::resetCGI() {
    cgiCtx.cgiPipeStdin = -1;
    cgiCtx.cgiPipeStdout = -1;
    cgiCtx.cgiPid = -1;
    cgiCtx.cgiOutput = "";
    cgiCtx.state = CgiContext::Forking;
}

void Connection::checkRoute() {
    if (!checkValidMethod(_request.method)) {
        setErrorResponse(_response, 501, RouteConfig());
        setState(Connection::SendResponse);
        return;
    }

    if (!checkMethodImplemented(_request.method)) {
        setErrorResponse(_response, 501, RouteConfig());
        setState(Connection::SendResponse);
        return;
    }

    if (route.hdlrs.empty()) {
        setErrorResponse(_response, 404, RouteConfig());
        setState(Connection::SendResponse);
        return;
    }

    if (route.hdlrs.find(_request.method) == route.hdlrs.end()) {
        setErrorResponse(_response, 405, RouteConfig());
        _state = Connection::SendResponse;
        return;
    }

    if (!route.cfg.redirect.second.empty()) {
        _state = Connection::Redirecting;
        return;
    }

    bool isCGIrequest = false;
    if (route.hdlrs.find("CGI") != route.hdlrs.end())
        isCGIrequest = checkCGIRequest(_request, route.cfg);

    if (isCGIrequest)
        _hdlr = route.hdlrs["CGI"];
    else
        _hdlr = route.hdlrs[getRequest().method];

    _state = Connection::Handling;
    return;
}

void Connection::resetResponse() {
    delete _response.body;
    delete _wrtr;
    _wrtr = NULL;
    _response = HttpResponse();
}

ssize_t Connection::sendResponse() {
    if (!_wrtr)
        _wrtr = new ResponseWriter(_response);

    _sendBuf.write(_wrtr);

    ssize_t r = _sendBuf.send(_sender, _fd);
    if (r <= 0)
        return r;

    if (_response.body && !_response.body->isDone())
        return r;

    _state = Reset;
    return r;
}

ssize_t Connection::readIntoBuf() { return _readBuf.recv(_fd); }

void Connection::parseBuf() {
    size_t count = 0;
    while (count < _readBuf.size()) {
        _prsr->feed(_readBuf.data() + count, 1);
        if (_prsr->error() || _prsr->ready()) {
            _readBuf.advance(count + 1);
            if (_prsr->ready()) {
                _request = _prsr->getRequest();
                _state = Routing;
            } else {
                _state = HandleBadRequest;
            }
            return;
        }
        count++;
    }
    _readBuf.clear();
}

std::string Connection::getReadBuf() { return std::string(_readBuf.data(), _readBuf.size()); }

int Connection::getFileDes() const { return _fd; }

Connection::STATE Connection::getState() const { return _state; }

void Connection::setState(Connection::STATE state) { _state = state; };

struct sockaddr_storage Connection::getAddr() const { return _addr; }
