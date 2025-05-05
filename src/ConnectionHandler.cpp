#include "ConnectionHandler.h"
#include "BadRequestHandler.h"
#include "HttpParser.h"
#include "IIONotifier.h"
#include "PingHandler.h"
#include "ResponseWriter.h"
#include "Router.h"
#include "logging.h"
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

ConnectionHandler::ConnectionHandler(ILogger& l, IIONotifier& ep) : _logger(l), _ioNotifier(ep) {}

ConnectionHandler::~ConnectionHandler(void) {
    for (std::map<int, Connection*>::iterator it = _connections.begin(); it != _connections.end(); it++) {
        delete it->second;
    }
}

void ConnectionHandler::_updateNotifier(Connection* conn) {
    int connfd = conn->getFileDes();
    switch (conn->getState()) {
    case Connection::ReadingHeaders:
        _ioNotifier.modify(connfd, READY_TO_READ);
        break;
    case Connection::Handling:
        _ioNotifier.modify(connfd, READY_TO_WRITE);
        break;
    case Connection::HandleBadRequest:
        _ioNotifier.modify(connfd, READY_TO_WRITE);
        break;
    case Connection::SendResponse:
        _ioNotifier.modify(connfd, READY_TO_WRITE);
        break;
    }
}

void ConnectionHandler::_addClientConnection(int connfd, struct sockaddr_storage theirAddr) {
    logConnection(_logger, theirAddr);
    Connection* conn = new Connection(theirAddr, connfd, new HttpParser(_logger));
    _connections[connfd] = conn;
    _ioNotifier.add(connfd);
}

void ConnectionHandler::_removeConnection(int connfd) {
    logDisconnect(_logger, _connections[connfd]->getAddr());
    delete _connections[connfd];
    _connections.erase(connfd);
    _ioNotifier.del(connfd);
}

void ConnectionHandler::_onClientHungUp(int connfd) { _removeConnection(connfd); }

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

void ConnectionHandler::_onSocketRead(int connfd) {
    Connection* conn = _connections[connfd];
    bool continueProcessing = true;
    while (continueProcessing) {
        HttpResponse resp;
        IHandler* hdlr;
        Connection::STATE currentState = _connections[connfd]->getState();
        switch (currentState) {
        case Connection::ReadingHeaders:
            conn->readIntoBuf();
            conn->parseBuf();
            continueProcessing = (conn->getState() != currentState);
            break;
        case Connection::Handling:
            hdlr = new PingHandler();
            hdlr->handle(conn, {}, {});
            delete hdlr;
            continueProcessing = (conn->getState() != currentState);
            break;
        case Connection::HandleBadRequest:
            hdlr = new BadRequestHandler();
            hdlr->handle(conn, {}, {});
            delete hdlr;
            continueProcessing = (conn->getState() != currentState);
            break;
        default:
            continueProcessing = false;
            break;
        }
    }
    _updateNotifier(conn);
    return;
}

void ConnectionHandler::_onSocketWrite(int connfd) {
    Connection* conn = _connections[connfd];

    IResponseWriter* wrtr = new ResponseWriter(conn->_response);
    char buffer[1024];
    int bytesWritten = wrtr->write(buffer, 1024);
    send(connfd, buffer, bytesWritten, 0);
    delete wrtr;

    if (conn->_response.statusCode == 400) {
        _removeConnection(connfd);
    } else {
        conn->parseBuf();
        delete conn->_response.body;
        conn->_response.body = NULL;
        _updateNotifier(conn);
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
