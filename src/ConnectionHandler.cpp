#include "ConnectionHandler.h"
#include "HttpParser.h"
#include "IIONotifier.h"
#include "logging.h"
#include <errno.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

ConnectionHandler::ConnectionHandler(ILogger& l, IIONotifier& ep) : _logger(l), _ioNotifier(ep) {}

ConnectionHandler::~ConnectionHandler(void) {
    for (std::map<int, ConnectionInfo>::iterator it = _connections.begin(); it != _connections.end(); it++) {
        close(it->first);
        delete _parsers[it->first];
    }
}

void ConnectionHandler::_addClientConnection(int conn, struct sockaddr_storage theirAddr) {
    logConnection(_logger, theirAddr);
    ConnectionInfo connInfo;
    connInfo.addr = theirAddr;
    connInfo.fd = conn;
    _connections[conn] = connInfo;
    _parsers[conn] = new HttpParser(_logger);
    _ioNotifier.add(conn);
}

void ConnectionHandler::_removeClientConnection(ConnectionInfo connInfo) {
    _connections.erase(connInfo.fd);
    delete _parsers[connInfo.fd];
    _parsers.erase(connInfo.fd);
    _ioNotifier.del(connInfo.fd);
    close(connInfo.fd);
    logDisconnect(_logger, connInfo.addr);
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

void ConnectionHandler::_readPipeline(int conn) {
    IHttpParser* prsr = _parsers.at(conn);
    char buffer[1024];
    ssize_t r = recv(conn, buffer, 1024, 0);
    buffer[r] = '\0';
    prsr->feed(buffer, r);
    std::string msg = std::string(buffer);
    if (prsr->error()) {
        _responses[conn] = "HTTP/1.1 400 Bad Request\r\n"
                           "\r\n";

        _ioNotifier.modify(conn, READY_TO_WRITE);
    }
    if (prsr->ready()) {
        _responses[conn] = "HTTP/1.1 200 OK\r\n"
                           "Content-Length: 4\r\n"
                           "\r\n"
                           "pong";
        _ioNotifier.modify(conn, READY_TO_WRITE);
    }
    return;
}

int ConnectionHandler::handleConnection(int fd, e_notif notif) {
    try {
        ConnectionInfo connInfo = _connections.at(fd);
        switch (notif) {
        case READY_TO_READ:
            _readPipeline(fd);
            break;
        case READY_TO_WRITE:
            send(fd, _responses[fd].c_str(), _responses[fd].length(), 0);
            _ioNotifier.modify(fd, READY_TO_READ);
            break;
        case CLIENT_HUNG_UP:
            _removeClientConnection(connInfo);
            break;
        case BROKEN_CONNECTION:
            break;
        }
        return fd;
    } catch (std::out_of_range& e) {
        return _acceptNewConnection(fd);
    }
}
