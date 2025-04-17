#include "Listener.h"
#include "logging.h"
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
    int count = 0;
    while (_isListening) {
        int ready = epoll_wait(_epfd, events, 1, 10);
        if (ready == 0)
            continue;
        if (!_isListening)
            break;

        int portfd = events[0].data.fd;
        if (portfd == _portfds[0]) {
            int conn = accept(portfd, (struct sockaddr*)&theirAddr, (socklen_t*)&addrlen);
            if (conn < 0) {
                _logger->log("ERROR", "accept: " + std::string(strerror(errno)));
                exit(1);
            }
            logConnection(_logger, theirAddr);

            struct epoll_event event;
            event.events = EPOLLRDHUP; // Monitor for read events
            event.data.fd = conn; // Monitor for read events
            epoll_ctl(_epfd, EPOLL_CTL_ADD, conn, &event);
            _activeConns.push_back(conn);
            continue;
        }
 
        epoll_ctl(_epfd, EPOLL_CTL_DEL, portfd, NULL);
        _logger->log("INFO", "Disconnect IP: 127.0.0.2, Port: 12345");
        close(portfd);
        count++;
    }
}

void Listener::stop() {
    _isListening = false;
    if (close(_epfd) == -1) {
        _logger->log("ERROR", "close: " + std::string(strerror(errno)));
        exit(1);
    }
}

void Listener::add(int portfd) {
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = portfd;
    epoll_ctl(_epfd, EPOLL_CTL_ADD, portfd, &event);
    _portfds.push_back(portfd);
}
