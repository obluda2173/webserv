#include "ConnectionHandler.h"
#include "BadRequestHandler.h"
#include "BodyParser.h"
#include "HttpParser.h"
#include "IIONotifier.h"
#include "Router.h"
#include "handlerUtils.h"
#include "logging.h"
#include "utils.h"
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

ConnectionHandler::ConnectionHandler(std::map< std::string, IRouter* > routers, ILogger& l, IIONotifier& io)
    : _routers(routers), _logger(l), _ioNotifier(io) {
    _bodyPrsr = new BodyParser();
}

ConnectionHandler::~ConnectionHandler(void) {
    delete _bodyPrsr;
    for (std::map< int, Connection* >::iterator it = _connections.begin(); it != _connections.end(); it++)
        delete it->second;

    for (std::map< std::string, IRouter* >::iterator it = _routers.begin(); it != _routers.end(); it++)
        delete it->second;
}

void ConnectionHandler::_updateNotifier(Connection* conn) {
    int connfd = conn->getFileDes();
    switch (conn->getState()) {
    case Connection::ReadingHeaders:
        _ioNotifier.modify(connfd, READY_TO_READ);
        break;
    case Connection::Handling:
        _ioNotifier.modify(connfd, READY_TO_READ);
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
    default:
        break;
    }
}

void ConnectionHandler::_addClientConnection(int connfd, struct sockaddr_storage theirAddr, std::string addrPort) {
    logConnection(_logger, theirAddr);
    Connection* conn = new Connection(theirAddr, connfd, addrPort, new HttpParser(_logger));
    _connections[connfd] = conn;
    _ioNotifier.add(connfd);
}

void ConnectionHandler::_removeConnection(int connfd) {
    logDisconnect(_logger, _connections[connfd]->getAddr());
    delete _connections[connfd];
    _connections.erase(connfd);
    _ioNotifier.del(connfd);
}

int ConnectionHandler::_acceptNewConnection(int socketfd) {
    struct sockaddr_storage theirAddr;
    int addrlen = sizeof(theirAddr);
    int fd = accept(socketfd, (struct sockaddr*)&theirAddr, (socklen_t*)&addrlen);
    if (fd < 0) {
        _logger.log("ERROR", "accept: " + std::string(strerror(errno)));
        return -1;
    }
    _addClientConnection(fd, theirAddr, getAddressAndPort(socketfd));
    return fd;
}

void ConnectionHandler::_handleState(Connection* conn) {
    // _logger.log("INFO", "Handle state: " + to_string(conn->getFileDes()));
    // std::cout << std::string(conn->_readBuf.data(), conn->_readBuf.size()) << std::endl;
    HttpResponse resp;
    IHandler* hdlr;
    IRouter* router;
    Route route;
    bool isCGIrequest;
    bool continueProcessing = true;
    while (continueProcessing) {
        Connection::STATE currentState = conn->getState();
        switch (currentState) {
        case Connection::ReadingHeaders:
            conn->parseBuf();
            continueProcessing = (conn->getState() != currentState);
            break;
        case Connection::Routing:
            router = _routers[conn->getAddrPort()];
            conn->_request.uri = normalizePath("", conn->_request.uri);
            route = router->match(conn->getRequest());
            // redirection should probably be here (checking route.cfg.redirect)
            // possible state change: SetRedirectResponse and SendResponse
            if (route.hdlrs.find(conn->_request.method) == route.hdlrs.end()) {
                setErrorResponse(conn->_response, 404, "Not Found", RouteConfig());
                conn->setState(Connection::SendResponse);
                break;
            }
            isCGIrequest = false;
            if (route.hdlrs.find("CGI") != route.hdlrs.end())
                isCGIrequest = checkCGIRequest(conn->_request, route.cfg);

            if (isCGIrequest)
                conn->_hdlr = conn->route.hdlrs["CGI"];
            else
                conn->_hdlr = route.hdlrs[conn->getRequest().method];

            conn->route = route;
            conn->setState(Connection::Handling);
            break;
        case Connection::Handling:
            _bodyPrsr->parse(conn);
            conn->_hdlr->handle(conn, conn->_request, conn->route.cfg);
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

void ConnectionHandler::_onSocketRead(Connection* conn) {
    conn->readIntoBuf();
    if (conn->_readBuf.size() == 0) // EOF
        return _removeConnection(conn->getFileDes());
    _handleState(conn);
    return;
}

void ConnectionHandler::_onSocketWrite(Connection* conn) {
    if (conn->getState() == Connection::HandlingCgi) {
        conn->_hdlr->handle(conn, conn->_request, conn->route.cfg);
    }

    if (conn->getState() == Connection::SendResponse) {
        conn->sendResponse();

        if (conn->_response.statusCode == 400) {
            _removeConnection(conn->getFileDes());
            return;
        }
        if (conn->getState() == Connection::Reset) {
            conn->resetResponse();
            conn->setState(Connection::ReadingHeaders);
            _handleState(conn); // possibly data inside Connection
            return;
        }
    }
}

int ConnectionHandler::handleConnection(int fd, e_notif notif) {
    _logger.log("INFO", "Handling fd: " + to_string(fd));
    if (_connections.find(fd) == _connections.end()) {
        return _acceptNewConnection(fd);
    }

    Connection* conn = _connections[fd];
    switch (notif) {
    case READY_TO_READ:
        _logger.log("INFO", "Ready to read");
        _onSocketRead(conn);
        break;
    case READY_TO_WRITE:
        _logger.log("INFO", "Ready to write");
        _onSocketWrite(conn);
        break;
    case CLIENT_HUNG_UP:
        _logger.log("INFO", "Client hung up");
        _removeConnection(fd);
        break;
    case BROKEN_CONNECTION:
        _logger.log("INFO", "Broken Connection");
        _removeConnection(fd);
        break;
    case TIMEOUT:
        _logger.log("INFO", "Timeout");
        _removeConnection(fd);
        break;
    }
    return fd;
}
