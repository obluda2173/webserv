#include "Server.h"
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

Server::Server(ILogger *l) : _logger(l) { _logger->log("INFO", "Server constructed"); }

bool Server::isRunning() const { return _isRunning; }

void Server::start() {
    _logger->log("INFO", "Server is starting...");
    _isRunning = true;
    _serverfd = socket(PF_INET, SOCK_STREAM, 0);

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, "8080", &hints, &res);
    freeaddrinfo(res);
    // socket(res->ai_family, res->ai_addr, res->ai_addrlen);
    // bind(_serverfd, *res);
    _logger->log("INFO", "Server started");
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
