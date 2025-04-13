#include "Server.h"
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

Server::Server(ILogger *l) : _logger(l) { _logger->log("INFO", "Server constructed"); }

bool Server::isRunning() const { return _isRunning; }

void Server::start() {
    _logger->log("INFO", "Server is starting...");

    _serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverfd < 0) {
        perror("socket");
        exit(1);
    }

    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(_serverfd, (sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(_serverfd, 5) < 0) {
        perror("listen");
        exit(1);
    }

    _isRunning = true;
    _logger->log("INFO", "Server started");

    // int clientfd = accept(_serverfd, nullptr, nullptr);
    // close(clientfd);
}

void Server::stop() {
    _logger->log("INFO", "Server is stopping...");

    // this will free the the port
    int yes = 1;
    if (setsockopt(_serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    _isRunning = false;
    _logger->log("INFO", "Server stopped");
}
