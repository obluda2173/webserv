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

Listener::Listener() : _portfd(-1) {}

Listener::Listener(int &portfd, ILogger *logger) : _portfd(portfd), _logger(logger) {}

void Listener::listen() {
    int epfd = epoll_create(1);
    if (epfd == -1) {
        _logger->log("ERROR", "epoll_create: " + std::string(strerror(errno)));
        exit(1);
    }
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = _portfd;

    epoll_ctl(epfd, EPOLL_CTL_ADD, _portfd, &event);
    struct epoll_event events[1];
    _isListening = true;
    while (_isListening) {
        int conn;
        std::stringstream info;
        struct sockaddr_in theirAddr;
        int addrlen = sizeof(theirAddr);
        int ready = epoll_wait(epfd, events, 1, 10);
        if (ready == 0)
            continue;
        if (!_isListening)
            break;
        conn = accept(_portfd, (struct sockaddr *)&theirAddr, (socklen_t *)&addrlen);
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
    close(epfd);
}

void Listener::stop() { _isListening = false; }
