#include "Listener.h"
#include "logging.h"
#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sstream>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

Listener::Listener(ILogger* logger) : _logger(logger) {
    _epfd = epoll_create(1);
    if (_epfd == -1) {
        _logger->log("ERROR", "epoll_create: " + std::string(strerror(errno)));
        exit(1);
    }
}

Listener::~Listener() { close(_epfd); };

void Listener::listen() {
    int conn;
    int ready;
    std::stringstream info;
    struct sockaddr_in theirAddr;
    int addrlen = sizeof(theirAddr);
    struct epoll_event events[1];

    _isListening = true;
    while (_isListening) {
        ready = epoll_wait(_epfd, events, 1, 10);
        if (ready == 0)
            continue;
        if (!_isListening)
            break;

        int portfd = events[0].data.fd;
        conn = accept(portfd, (struct sockaddr*)&theirAddr, (socklen_t*)&addrlen);
        if (conn < 0) {
            _logger->log("ERROR", "accept: " + std::string(strerror(errno)));
            exit(1);
        }

        logConnection(_logger, theirAddr);

        // close connection
        if (close(conn) == -1) {
            _logger->log("ERROR", "close: " + std::string(strerror(errno)));
            exit(1);
        }
    }
}

void Listener::stop() { _isListening = false; }

void Listener::add(int portfd) {
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = portfd;
    epoll_ctl(_epfd, EPOLL_CTL_ADD, portfd, &event);
    _portfds.push_back(portfd);
}
