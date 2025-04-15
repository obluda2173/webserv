#include "Server.h"
#include "ILogger.h"
#include "logging.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sstream>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

Server::Server(ILogger *l) : _logger(l) { _logger->log("INFO", "Server constructed"); }

bool Server::isRunning() const { return _isRunning; }

void Server::start() {
    _logger->log("INFO", "Server is starting...");

    _serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverfd < 0) {
        _logger->log("ERROR", "socket: " + std::string(strerror(errno)));
        exit(1);
    }

    sockaddr_in svrAddr = {};
    svrAddr.sin_family = AF_INET;
    svrAddr.sin_addr.s_addr = INADDR_ANY;
    svrAddr.sin_port = htons(8080);

    int yes = 1;
    if (setsockopt(_serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        _logger->log("ERROR", "setsockopt: " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }
    if (bind(_serverfd, (sockaddr *)&svrAddr, sizeof(svrAddr)) < 0) {
        _logger->log("ERROR", "bind: " + std::string(strerror(errno)));
        exit(1);
    }
    if (listen(_serverfd, 5) < 0) {
        _logger->log("ERROR", "listen: " + std::string(strerror(errno)));
        exit(1);
    }

    _isRunning = true;
    _logger->log("INFO", "Server started");
    _listenPoll();
}

void Server::_listenEPoll(void) {
    int epfd = epoll_create(0);
    if (epfd == -1) {
        _logger->log("ERROR", "epoll_create: " + std::string(strerror(errno)));
    }
}

void Server::_listenPoll(void) {
    struct pollfd serverPfd = {_serverfd, POLLIN | POLLHUP, 0};
    while (_isRunning) {
        int conn;
        std::stringstream info;
        struct sockaddr_in theirAddr;
        int addrlen = sizeof(theirAddr);
        int ready = poll(&serverPfd, 1, 10);
        if (!_isRunning)
            break;
        if (ready == 0)
            continue;
        conn = accept(_serverfd, (struct sockaddr *)&theirAddr, (socklen_t *)&addrlen);
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

void Server::stop() {
    _logger->log("INFO", "Server is stopping...");
    _isRunning = false;
    if (close(_serverfd) == -1) {
        _logger->log("ERROR", "close: " + std::string(strerror(errno)));
        exit(1);
    }
    _logger->log("INFO", "Server stopped");
}
