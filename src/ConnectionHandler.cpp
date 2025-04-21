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

void ConnectionHandler::_addClientConnection(int conn, struct sockaddr_storage* theirAddr) {

    // char ipstr[INET6_ADDRSTRLEN]; /* to store the ip-address */
    // struct addrinfo* addrInfo;
    // getSvrAddrInfo(clientIp.c_str(), clientPort.c_str(), &addrInfo);

    // void* addr;
    // const char* ipver;
    // struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)addrInfo->ai_addr;
    // addr = &(ipv6->sin6_addr);
    // ipver = "IPv6";

    // inet_ntop(addrInfo->ai_family, addr, ipstr, sizeof ipstr);
    // printf("  %s: %s\n", ipver, ipstr);
    if (theirAddr->ss_family == AF_INET) {
        struct sockaddr_in* theirAddrIpv4 = reinterpret_cast<struct sockaddr_in*>(theirAddr);
        logConnection(_logger, *theirAddrIpv4);
        ConnectionInfo connInfo;
        connInfo.addr = *theirAddrIpv4;
        connInfo.type = CLIENT_SOCKET;
        connInfo.fd = conn;
        _connections[conn] = connInfo;
    }
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
