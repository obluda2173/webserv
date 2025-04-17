#include "Listener.h"
#include "logging.h"
#include <algorithm>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <sstream>
#include <sys/epoll.h>

Listener::Listener(ILogger* logger) : _logger(logger) {
    _epfd = epoll_create(1);
    if (_epfd == -1) {
        _logger->log("ERROR", "epoll_create: " + std::string(strerror(errno)));
        exit(1);
    }
}

Listener::~Listener() { close(_epfd); };

void Listener::listen() {
    std::stringstream info;
    struct sockaddr_in theirAddr;
    int addrlen = sizeof(theirAddr);
    struct epoll_event events[1];

    _isListening = true;
    while (_isListening) {
        int ready = epoll_wait(_epfd, events, 1, 10);
        if (ready == 0)
            continue;
        if (!_isListening)
            break;

        // int portfd = events[0].data.fd; // this is becoming difficult because it can be either a portfd or a
        // connection
        ConnectionInfo* connInfo = (ConnectionInfo*)events[0].data.ptr;
        int portfd = connInfo->fd;
        if (std::find(_portfds.begin(), _portfds.end(), portfd) != std::end(_portfds)) {
            int conn = accept(portfd, (struct sockaddr*)&theirAddr, (socklen_t*)&addrlen);
            if (conn < 0) {
                _logger->log("ERROR", "accept: " + std::string(strerror(errno)));
                exit(1);
            }
            logConnection(_logger, theirAddr);

            ConnectionInfo* connInfo = new ConnectionInfo;
            connInfo->addr = theirAddr;
            connInfo->fd = conn;
            struct epoll_event event;
            event.events = EPOLLRDHUP; // Monitor for read events
            event.data.ptr = connInfo;
            epoll_ctl(_epfd, EPOLL_CTL_ADD, conn, &event);
            _activeConns.push_back(conn);
            continue;
        };

        // ConnectionInfo* connInfo = (ConnectionInfo*)events[0].data.ptr;

        epoll_ctl(_epfd, EPOLL_CTL_DEL, connInfo->fd, NULL);

        theirAddr = connInfo->addr;
        unsigned char* ip = reinterpret_cast<unsigned char*>(&theirAddr.sin_addr.s_addr);
        info << "Disconnect IP: " << static_cast<int>(ip[0]) << '.' << static_cast<int>(ip[1]) << '.'
             << static_cast<int>(ip[2]) << '.' << static_cast<int>(ip[3]) << ", Port: " << ntohs(theirAddr.sin_port);
        _logger->log("INFO", info.str());
        info.str("");
        close(connInfo->fd);
        delete connInfo;
    }
}

void Listener::stop() {
    _isListening = false;
    if (close(_epfd) == -1) {
        _logger->log("ERROR", "close: " + std::string(strerror(errno)));
        exit(1);
    }
    for (size_t i = 0; i < _portfds_infos.size(); i++)
        delete _portfds_infos[i];
}

void Listener::add(int portfd) {

    ConnectionInfo* connInfo = new ConnectionInfo;
    connInfo->fd = portfd;

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.ptr = connInfo;
    epoll_ctl(_epfd, EPOLL_CTL_ADD, portfd, &event);
    _portfds.push_back(portfd);
    _portfds_infos.push_back(connInfo);
}
