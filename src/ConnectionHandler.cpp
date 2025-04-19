#include "ConnectionHandler.h"
#include "logging.h"
#include <netinet/in.h>
#include <stdexcept>
#include <string.h>

ConnectionHandler::ConnectionHandler(ILogger* l, EPollManager* ep) : _logger(l), _epollMngr(ep) {}

ConnectionHandler::~ConnectionHandler(void) {
    for (std::map<int, ConnectionInfo>::iterator it = _connections.begin(); it != _connections.end(); it++) {
        close(it->first);
    }
}

void ConnectionHandler::_addClientConnection(int conn, struct sockaddr* theirAddr) {
    struct sockaddr_in* theirAddrIpv4 = reinterpret_cast<struct sockaddr_in*>(theirAddr);
    logConnection(_logger, *theirAddrIpv4);
    ConnectionInfo connInfo;
    connInfo.addr = *theirAddrIpv4;
    connInfo.type = CLIENT_SOCKET;
    connInfo.fd = conn;
    _connections[conn] = connInfo;
    _epollMngr->add(conn, CLIENT_HUNG_UP);
}

void ConnectionHandler::_removeClientConnection(ConnectionInfo connInfo) {
    close(connInfo.fd);
    _epollMngr->del(connInfo.fd);
    _connections.erase(connInfo.fd);
    logDisconnect(_logger, connInfo.addr);
}

void ConnectionHandler::_acceptNewConnection(int socketfd) {
    struct sockaddr theirAddr;
    int addrlen = sizeof(theirAddr);
    int conn = accept(socketfd, &theirAddr, (socklen_t*)&addrlen);
    if (conn < 0) {
        _logger->log("ERROR", "accept: " + std::string(strerror(errno)));
        exit(1);
    }
    _addClientConnection(conn, &theirAddr);
}

void ConnectionHandler::handleConnection(int fd) {
    try {
        ConnectionInfo connInfo = _connections.at(fd);
        _removeClientConnection(connInfo);
    } catch (std::out_of_range& e) {
        _acceptNewConnection(fd);
    }
}
