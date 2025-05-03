#include "ConnectionHandler.h"
#include "HttpParser.h"
#include "IIONotifier.h"
#include "logging.h"
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

ConnectionHandler::ConnectionHandler(ILogger& l, IIONotifier& ep) : _logger(l), _ioNotifier(ep) {}

ConnectionHandler::~ConnectionHandler(void) {
    for (std::map<int, Connection*>::iterator it = _connections.begin(); it != _connections.end(); it++) {
        close(it->first);
        delete _parsers[it->first];
        delete it->second;
    }
}

void ConnectionHandler::_addClientConnection(int conn, struct sockaddr_storage theirAddr) {
    logConnection(_logger, theirAddr);
    Connection* connInfo = new Connection(theirAddr, conn);
    // connInfo.addr = theirAddr;
    // connInfo.fd = conn;
    _connections[conn] = connInfo;
    _parsers[conn] = new HttpParser(_logger);
    _responses[conn] = HttpResponse{0, "", "", false, "", "", ""};
    _ioNotifier.add(conn);
}

void ConnectionHandler::_removeClientConnection(int conn) {
    Connection* connInfo = _connections[conn];
    _connections.erase(connInfo->fd);
    _responses.erase(connInfo->fd); // TODO: secure all the raises agains exceptions being thrown
    delete _parsers[connInfo->fd];
    _parsers.erase(connInfo->fd);
    _ioNotifier.del(connInfo->fd);
    close(connInfo->fd);
    logDisconnect(_logger, connInfo->addr);
    delete connInfo;
}

int ConnectionHandler::_acceptNewConnection(int socketfd) {
    struct sockaddr_storage theirAddr;
    int addrlen = sizeof(theirAddr);
    int conn = accept(socketfd, (struct sockaddr*)&theirAddr, (socklen_t*)&addrlen);
    if (conn < 0) {
        _logger.log("ERROR", "accept: " + std::string(strerror(errno)));
        exit(1);
    }
    _addClientConnection(conn, theirAddr);
    return conn;
}

void ConnectionHandler::_readFromConn(Connection* connInfo) {
    char buf[1024];
    ssize_t r = recv(connInfo->fd, buf, 1024, 0);
    buf[r] = '\0';
    connInfo->buf += buf;
}

void ConnectionHandler::_onSocketRead(int conn, bool withRead) {
    Connection* connInfo = _connections[conn];
    IHttpParser* prsr = _parsers.at(conn);
    if (withRead)
        _readFromConn(connInfo);
    char* b = (char*)connInfo->buf.c_str();
    while (*b) {
        prsr->feed(b, 1);
        if (prsr->error() || prsr->ready()) {
            _ioNotifier.modify(conn, READY_TO_WRITE);
            connInfo->buf = b + 1;
            return;
        }
        b++;
    }
    connInfo->buf = b;
    _ioNotifier.modify(conn, READY_TO_READ);
    return;
}

void ConnectionHandler::_sendPipeline(int conn) {
    IHttpParser* prsr = _parsers.at(conn);
    if (prsr->error()) {
        _responses[conn].response += "HTTP/1.1 400 Bad Request\r\n"
                                     "\r\n";

        _responses[conn].statusCode = 400;
    }
    if (prsr->ready()) {
        _responses[conn].response += "HTTP/1.1 200 OK\r\n"
                                     "Content-Length: 4\r\n"
                                     "\r\n"
                                     "pong";
        _responses[conn].statusCode = 200;
    }
    prsr->resetPublic();

    send(conn, _responses[conn].response.c_str(), _responses[conn].response.length(), 0);

    if (_responses[conn].statusCode == 400) {
        _removeClientConnection(conn);
    } else {
        _responses[conn].response.clear();
        if (!_connections[conn]->buf.empty()) {
            _onSocketRead(conn, false);
            return;
        }
        _ioNotifier.modify(conn, READY_TO_READ);
    }
}

int ConnectionHandler::handleConnection(int fd, e_notif notif) {
    if (_connections.find(fd) == _connections.end())
        return _acceptNewConnection(fd);

    switch (notif) {
    case READY_TO_READ:
        _onSocketRead(fd, true);
        break;
    case READY_TO_WRITE:
        _sendPipeline(fd);
        break;
    case CLIENT_HUNG_UP:
        _removeClientConnection(fd);
        break;
    case BROKEN_CONNECTION:
        break;
    }
    return fd;
}
