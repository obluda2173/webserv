#include "Server.h"
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
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

    int yes = 1;
    if (setsockopt(_serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
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

    struct sockaddr_in theirAddr;
    int addrlen = sizeof(theirAddr);
    while (_isRunning) {
        int clientfd = accept(_serverfd, (struct sockaddr *)&theirAddr, (socklen_t *)&addrlen);
        _logger->log("INFO", "Connection accepted from IP: 127.0.0.2, Port: 8081");
        std::cout << "hello" << std::endl;
        close(clientfd);
    }
}

void Server::stop() {
    _logger->log("INFO", "Server is stopping...");
    if (close(_serverfd) == -1) {
        perror("close");
        exit(1);
    }
    _isRunning = false;
    _logger->log("INFO", "Server stopped");
}
