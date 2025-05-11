#include "ConnectionHandler.h"
#include "BadRequestHandler.h"
#include "HttpParser.h"
#include "IIONotifier.h"
#include "logging.h"
#include "utils.h"
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
    conn->logState(&_logger);
    return;
}

void ConnectionHandler::_onSocketWrite(int connfd) {
    Connection* conn = _connections[connfd];

    std::cout << "hello" << std::endl;
    conn->sendResponse();
    conn->logState(&_logger);
    if (conn->_response.statusCode == 400) {
        std::cout << "gone here instead" << std::endl;
        _removeConnection(connfd);
        return;
    }
    if (conn->getState() == Connection::Reset) {
        std::cout << "gone here" << std::endl;
        conn->resetResponse();
        conn->setState(Connection::ReadingHeaders);
        _handleState(conn); // possibly data inside Connection
        return;
    }
    std::cout << "in fact, I've gone here instead" << std::endl;
}

int ConnectionHandler::handleConnection(int fd, e_notif notif) {
    if (_connections.find(fd) == _connections.end())
        return _acceptNewConnection(fd);

    switch (notif) {
    case READY_TO_READ:
        _logger.log("DEBUG", "handleConnection notif: READY_TO_READ " + to_string(fd));
        _onSocketRead(fd);
        break;
    case READY_TO_WRITE:
        _logger.log("DEBUG", "handleConnection notif: READY_TO_WRITE " + to_string(fd));
        _onSocketWrite(fd);
        break;
    case CLIENT_HUNG_UP:
        _logger.log("DEBUG", "handleConnection notif: CLIENT_HUNG_UP " + to_string(fd));
        _onClientHungUp(fd);
        break;
    case BROKEN_CONNECTION:
        _logger.log("DEBUG", "handleConnection notif: BROKEN_CONNECTION " + to_string(fd));
        break;
    }
    return fd;
}
