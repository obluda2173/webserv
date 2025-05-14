#include "ConnectionHandler.h"
#include "BadRequestHandler.h"
#include "HttpParser.h"
#include "IIONotifier.h"
#include "logging.h"
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

ConnectionHandler::ConnectionHandler(IRouter* router, ILogger& l, IIONotifier& ep)
    : _router(router), _logger(l), _ioNotifier(ep) {}

ConnectionHandler::~ConnectionHandler(void) {
    for (std::map< int, Connection* >::iterator it = _connections.begin(); it != _connections.end(); it++) {
        delete it->second;
    }
    delete _router;
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
    case Connection::Reset:
        _ioNotifier.modify(connfd, READY_TO_READ);
        break;
    case Connection::HandlingCgi:
        _ioNotifier.modify(conn->cgiCtx.cgiPipeFd, READY_TO_READ);
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
        return -1;
    }
    _addClientConnection(fd, theirAddr);
    return fd;
}

void ConnectionHandler::_handleState(Connection* conn) {
    bool continueProcessing = true;
    while (continueProcessing) {
        HttpResponse resp;
        IHandler* hdlr;
        Route route;
        Connection::STATE currentState = conn->getState();
        switch (currentState) {
        case Connection::ReadingHeaders:
            conn->parseBuf();
            continueProcessing = (conn->getState() != currentState);
            break;
        case Connection::Handling:
            route = _router->match(conn->getRequest());
            route.hdlrs[conn->getRequest().method]->handle(conn, conn->_request, route.cfg);
            continueProcessing = (conn->getState() != currentState);
            break;
        case Connection::HandleBadRequest:
            hdlr = new BadRequestHandler();
            hdlr->handle(conn, HttpRequest(), RouteConfig());
            delete hdlr;
            continueProcessing = (conn->getState() != currentState);
            break;
        default:
            continueProcessing = false;
            break;
        }
    }

    _updateNotifier(conn);
}

void ConnectionHandler::_onSocketRead(int connfd) {
    Connection* conn = _connections[connfd];
    conn->readIntoBuf();
    _handleState(conn);
    return;
}

void ConnectionHandler::_onSocketWrite(int connfd) {
    Connection* conn = _connections[connfd];

    conn->sendResponse();

    if (conn->_response.statusCode == 400) {
        _removeConnection(connfd);
        return;
    }
    if (conn->getState() == Connection::Reset) {
        conn->resetResponse();
        conn->setState(Connection::ReadingHeaders);
        _handleState(conn); // possibly data inside Connection
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
