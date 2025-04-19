#include "ConnectionHandler.h"
#include "logging.h"
#include <netinet/in.h>
#include <string.h>

ConnectionHandler::ConnectionHandler(ILogger* l, EPollManager* ep) : _logger(l), _epollMngr(ep) {}

ConnectionHandler::~ConnectionHandler(void) {}

void ConnectionHandler::_addClientConnection(int conn, struct sockaddr* theirAddr) {
    struct sockaddr_in* theirAddrIpv4 = reinterpret_cast<struct sockaddr_in*>(theirAddr);
    logConnection(_logger, *theirAddrIpv4);
    ConnectionInfo* connInfo = new ConnectionInfo;
    connInfo->addr = *theirAddrIpv4;
    connInfo->type = CLIENT_SOCKET;
    connInfo->fd = conn;
    _epollMngr->add(conn, connInfo, CLIENT_HUNG_UP);
}

void ConnectionHandler::_removeClientConnection(ConnectionInfo* connInfo) {
    // check for connInfo=NULL
    logDisconnect(_logger, connInfo->addr);
    _epollMngr->del(connInfo->fd);
    close(connInfo->fd);
    delete connInfo;
}

void ConnectionHandler::_acceptNewConnection(ConnectionInfo* connInfo) {
    struct sockaddr theirAddr;
    int addrlen = sizeof(theirAddr);
    int conn = accept(connInfo->fd, &theirAddr, (socklen_t*)&addrlen);
    if (conn < 0) {
        _logger->log("ERROR", "accept: " + std::string(strerror(errno)));
        exit(1);
    }
    _addClientConnection(conn, &theirAddr);
}

void ConnectionHandler::handleConnection(ConnectionInfo* connInfo) {
    if (connInfo->type == PORT_SOCKET) {
        _acceptNewConnection(connInfo);
        return;
    };

    _removeClientConnection(connInfo);
}
