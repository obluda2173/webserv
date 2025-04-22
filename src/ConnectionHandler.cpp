#include "ConnectionHandler.h"
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
    for (std::map<int, ConnectionInfo>::iterator it = _connections.begin(); it != _connections.end(); it++)
        close(it->first);
}

void ConnectionHandler::_addClientConnection(int conn, struct sockaddr_storage theirAddr) {
    logConnection(_logger, theirAddr);
    ConnectionInfo connInfo;
    connInfo.addr = theirAddr;
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

int ConnectionHandler::handleConnection(int fd, e_notif notif) {
    try {
        ConnectionInfo connInfo = _connections.at(fd);
        if (notif == READY_TO_READ) {
            char buffer[1024];
            ssize_t r = recv(fd, buffer, 1024, 0);
            buffer[r] = '\0';
            std::string msg = std::string(buffer);
            if (msg.find("ping", 0) != std::string::npos) {
                _response = "HTTP/1.1 200 OK\r\n"
                            "Content-Length: 4\r\n"
                            "\r\n"
                            "pong";
            } else {
                _response = "HTTP/1.1 400 Bad Request\r\n"
                            "\r\n";
            }
        } else if (notif == CLIENT_HUNG_UP) {
            _removeClientConnection(connInfo);
        } else {
            send(fd, _response.c_str(), _response.length(), 0);
        }
        return fd;
    } catch (std::out_of_range& e) {
        return _acceptNewConnection(fd);
    }
}
