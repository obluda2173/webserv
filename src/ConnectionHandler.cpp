#include "ConnectionHandler.h"
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
    }
}

void ConnectionHandler::_addClientConnection(int conn, struct sockaddr_storage theirAddr) {
    logConnection(_logger, theirAddr);
    ConnectionInfo connInfo;
    connInfo.addr = theirAddr;
    connInfo.type = CLIENT_SOCKET;
    connInfo.fd = conn;
    _connections[conn] = connInfo;
    _ioNotifier.add(conn, CLIENT_HUNG_UP);
}

void ConnectionHandler::_removeClientConnection(ConnectionInfo connInfo) {
    close(connInfo.fd);
    _ioNotifier.del(connInfo.fd);
    _connections.erase(connInfo.fd);
    logDisconnect(_logger, connInfo.addr);
}

void ConnectionHandler::_acceptNewConnection(int socketfd) {
    struct sockaddr_storage theirAddr;
    int addrlen = sizeof(theirAddr);
    int conn = accept(socketfd, (struct sockaddr*)&theirAddr, (socklen_t*)&addrlen);
    if (conn < 0) {
        _logger.log("ERROR", "accept: " + std::string(strerror(errno)));
        exit(1);
    }
    _addClientConnection(conn, theirAddr);
}

void ConnectionHandler::handleConnection(int fd) {
    try {
        ConnectionInfo connInfo = _connections.at(fd);
        _removeClientConnection(connInfo);
    } catch (std::out_of_range& e) {
        _acceptNewConnection(fd);
    }
}
