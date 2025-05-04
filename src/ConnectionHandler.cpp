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
        delete it->second;
    }
}

void ConnectionHandler::_addClientConnection(int connfd, struct sockaddr_storage theirAddr) {
    logConnection(_logger, theirAddr);
    Connection* conn = new Connection(theirAddr, connfd, new HttpParser(_logger));
    _connections[connfd] = conn;
    _responses[connfd] = HttpResponse{0, "", "", false, "", "", ""};
    _ioNotifier.add(connfd);
}

void ConnectionHandler::_onClientHungUp(int connfd) {
    logDisconnect(_logger, _connections[connfd]->getAddr());
    delete _connections[connfd];
    _connections.erase(connfd);
    _responses.erase(connfd); // TODO: secure all the raises against exceptions being thrown
    _ioNotifier.del(connfd);
}

int ConnectionHandler::_acceptNewConnection(int socketfd) {
    struct sockaddr_storage theirAddr;
    int addrlen = sizeof(theirAddr);
    int fd = accept(socketfd, (struct sockaddr*)&theirAddr, (socklen_t*)&addrlen);
    if (fd < 0) {
        _logger.log("ERROR", "accept: " + std::string(strerror(errno)));
        exit(1); // TODO: you probably don't want to exit
    }
    _addClientConnection(fd, theirAddr);
    return fd;
}

void ConnectionHandler::_updateNotifier(int connfd) {
    switch (_connections[connfd]->getState()) {
    case Connection::ReadingHeaders:
        _ioNotifier.modify(connfd, READY_TO_READ);
        break;
    case Connection::WritingResponse:
        _ioNotifier.modify(connfd, READY_TO_WRITE);
        break;
    case Connection::WritingError:
        _ioNotifier.modify(connfd, READY_TO_WRITE);
        break;
    }
}

void ConnectionHandler::_onSocketRead(int connfd) {
    Connection* conn = _connections[connfd];
    switch (_connections[connfd]->getState()) {
    case Connection::ReadingHeaders:
        conn->readIntoBuf();
        conn->parseBuf();
        break;
    default:
        break;
    }
    _updateNotifier(connfd);
    return;
}

void ConnectionHandler::_onSocketWrite(int connfd) {
    Connection* conn = _connections[connfd];
    if (conn->getState() == Connection::WritingError) {
        _responses[connfd].response += "HTTP/1.1 400 Bad Request\r\n"
                                       "\r\n";

        _responses[connfd].statusCode = 400;
    }
    if (conn->getState() == Connection::WritingResponse) {
        _responses[connfd].response += "HTTP/1.1 200 OK\r\n"
                                       "Content-Length: 4\r\n"
                                       "\r\n"
                                       "pong";
        _responses[connfd].statusCode = 200;
    }

    send(connfd, _responses[connfd].response.c_str(), _responses[connfd].response.length(), 0);

    if (_responses[connfd].statusCode == 400) {
        _onClientHungUp(connfd);
    } else {
        _responses[connfd].response.clear();
        _connections[connfd]->parseBuf();
        _updateNotifier(connfd);
        return;
    }
}

int ConnectionHandler::handleConnection(int fd, e_notif notif) {
    if (_connections.find(fd) == _connections.end())
        return _acceptNewConnection(fd);

    switch (notif) {
    case READY_TO_READ:
        _onSocketRead(fd);
        break;
    case READY_TO_WRITE:
        _onSocketWrite(fd);
        break;
    case CLIENT_HUNG_UP:
        _onClientHungUp(fd);
        break;
    case BROKEN_CONNECTION:
        break;
    }
    return fd;
}
