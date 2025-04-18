#include "ConnectionHandler.h"
#include "logging.h"

ConnectionHandler::ConnectionHandler(ILogger* l, EPollManager* ep) : _logger(l), _epollMngr(ep) {}

ConnectionHandler::~ConnectionHandler(void) {}

void addClientConnection(ILogger* _logger, EPollManager* _epollMngr, int conn, struct sockaddr_in theirAddr) {
    logConnection(_logger, theirAddr);
    ConnectionInfo* connInfo = new ConnectionInfo;
    connInfo->addr = theirAddr;
    connInfo->type = CLIENT_SOCKET;
    connInfo->fd = conn;
    _epollMngr->add(conn, connInfo, CLIENT_HUNG_UP);
}

void removeClientConnection(ILogger* _logger, EPollManager* _epollMngr, ConnectionInfo* connInfo) {
    _epollMngr->del(connInfo->fd);
    logDisconnect(_logger, connInfo->addr);
    close(connInfo->fd);
    delete connInfo;
}

void ConnectionHandler::handleConnection(ConnectionInfo* connInfo) {

    struct sockaddr_in theirAddr;
    int addrlen = sizeof(theirAddr);
    if (connInfo->type == PORT_SOCKET) {
        int conn = accept(connInfo->fd, (struct sockaddr*)&theirAddr, (socklen_t*)&addrlen);
        if (conn < 0) {
            _logger->log("ERROR", "accept: " + std::string(strerror(errno)));
            exit(1);
        }
        addClientConnection(_logger, _epollMngr, conn, theirAddr);
        return;
    };

    removeClientConnection(_logger, _epollMngr, connInfo);
}
