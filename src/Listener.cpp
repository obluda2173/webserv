#include "Listener.h"
#include "EPollManager.h"
#include "logging.h"
#include <cstring>
#include <errno.h>
#include <netinet/in.h>
#include <sys/epoll.h>

Listener::Listener(ILogger* logger, EPollManager* epollMngr) : _logger(logger), _epollMngr(epollMngr) {}

Listener::~Listener() {}

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

void Listener::listen() {
    struct sockaddr_in theirAddr;
    int addrlen = sizeof(theirAddr);
    struct epoll_event events[1];

    _isListening = true;
    while (_isListening) {
        int ready = _epollMngr->wait(events, 1);
        if (ready == 0)
            continue;
        if (!_isListening)
            break;

        ConnectionInfo* connInfo = (ConnectionInfo*)events[0].data.ptr;
        if (connInfo->type == PORT_SOCKET) {
            int conn = accept(connInfo->fd, (struct sockaddr*)&theirAddr, (socklen_t*)&addrlen);
            if (conn < 0) {
                _logger->log("ERROR", "accept: " + std::string(strerror(errno)));
                exit(1);
            }
            addClientConnection(_logger, _epollMngr, conn, theirAddr);
            continue;
        };

        removeClientConnection(_logger, _epollMngr, connInfo);
    }
}

void Listener::stop() {
    _isListening = false;
    for (size_t i = 0; i < _portfds_infos.size(); i++)
        delete _portfds_infos[i];
}

void Listener::add(int portfd) {
    ConnectionInfo* connInfo = new ConnectionInfo;
    connInfo->fd = portfd;
    connInfo->type = PORT_SOCKET;

    _epollMngr->add(portfd, connInfo, READY_TO_READ);
    _portfds.push_back(portfd);
    _portfds_infos.push_back(connInfo);
}
