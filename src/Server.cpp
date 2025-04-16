#include "Server.h"
#include "ILogger.h"
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

Server::Server(ILogger* lo) : _logger(lo), _listener(new Listener(lo)) {}

Server::~Server() { delete _listener; }

bool Server::isRunning() const { return _isRunning; }

void Server::start() {
    _logger->log("INFO", "Server is starting...");

    int portfd;
    portfd = socket(AF_INET, SOCK_STREAM, 0);
    if (portfd < 0) {
        _logger->log("ERROR", "socket: " + std::string(strerror(errno)));
        exit(1);
    }

    sockaddr_in svrAddr = {};
    svrAddr.sin_family = AF_INET;
    svrAddr.sin_addr.s_addr = INADDR_ANY;
    svrAddr.sin_port = htons(8080);

    int yes = 1;
    if (setsockopt(portfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        _logger->log("ERROR", "setsockopt: " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }
    if (bind(portfd, (sockaddr*)&svrAddr, sizeof(svrAddr)) < 0) {
        _logger->log("ERROR", "bind: " + std::string(strerror(errno)));
        exit(1);
    }
    if (listen(portfd, 5) < 0) {
        _logger->log("ERROR", "listen: " + std::string(strerror(errno)));
        exit(1);
    }

    _portfds.push_back(portfd);

    _isRunning = true;
    _logger->log("INFO", "Server started");

    _listener->add(portfd);
    _listener->listen();
}

void Server::stop() {
    _logger->log("INFO", "Server is stopping...");
    _listener->stop();
    _isRunning = false;
    for (size_t i = 0; i < _portfds.size(); i++) {
        if (close(_portfds[i]) == -1) {
            _logger->log("ERROR", "close: " + std::string(strerror(errno)));
            exit(1);
        }
    }
    _logger->log("INFO", "Server stopped");
}
