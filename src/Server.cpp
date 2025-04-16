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

int new_socket(int port) {
    int _serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverfd < 0) {
        perror("socket");
        exit(1);
    }

    sockaddr_in svrAddr = {};
    svrAddr.sin_family = AF_INET;
    svrAddr.sin_addr.s_addr = INADDR_ANY;
    svrAddr.sin_port = htons(port);

    int yes = 1;
    if (setsockopt(_serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    if (bind(_serverfd, (sockaddr*)&svrAddr, sizeof(svrAddr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if (listen(_serverfd, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    return _serverfd;
}

Server::Server(ILogger* lo) : _logger(lo), _listener(new Listener(lo)) {}

Server::~Server() { delete _listener; }

bool Server::isRunning() const { return _isRunning; }

void Server::start() {
    _logger->log("INFO", "Server is starting...");

    _portfds.push_back(new_socket(8080));
    _listener->add(_portfds[0]);

    _isRunning = true;
    _logger->log("INFO", "Server started");
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
