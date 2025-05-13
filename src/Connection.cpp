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

#include "handlerUtils.h"
#include <sys/wait.h>
void Connection::handleCgiProcess() {
    ConnectionContext& ctx = this->ctx;

    if (ctx.cgiPipeFd != -1) {
        char buffer[4096];
        ssize_t count = read(ctx.cgiPipeFd, buffer, sizeof(buffer));
        if (count > 0) {
            ctx.cgiOutput.append(buffer, count);
        } else if (count == 0) { // EOF
            close(ctx.cgiPipeFd);
            ctx.cgiPipeFd = -1;
        } else if (count == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
            close(ctx.cgiPipeFd);
            ctx.cgiPipeFd = -1;
        }
    }

    int status;
    pid_t result = waitpid(ctx.cgiPid, &status, WNOHANG);
    if (result == -1) {
        setErrorResponse(_response, 500, "Internal Server Error", ctx.cgiRouteConfig);
        _state = SendResponse;
    } else if (result > 0) {
        if (ctx.cgiPipeFd != -1) {
            while (true) {
                char buffer[4096];
                ssize_t n = read(ctx.cgiPipeFd, buffer, sizeof(buffer));
                if (n > 0) ctx.cgiOutput.append(buffer, n);
                else if (n == 0) break;
                else if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                else break;
            }
            close(ctx.cgiPipeFd);
            ctx.cgiPipeFd = -1;
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            if (ctx.cgiOutput.empty()) {
                setErrorResponse(_response, 500, "Internal Server Error", ctx.cgiRouteConfig);
            } else {
                // Assuming _parseCgiOutput is handled or response is set directly
                setResponse(_response, 200, "OK", "text/php", ctx.cgiOutput.size(), new StringBodyProvider(ctx.cgiOutput));
            }
        } else {
            setErrorResponse(_response, 502, "Bad Gateway", ctx.cgiRouteConfig);
        }
        _state = SendResponse;
    }
}

int Connection::getFileDes() const { return _fd; }

Connection::STATE Connection::getState() const { return _state; }

struct sockaddr_storage Connection::getAddr() const { return _addr; }
