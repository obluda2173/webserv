#include "Server.h"
#include <netinet/in.h>
#include <sys/socket.h>

Server::Server(ILogger *l) : _logger(l) { _logger->log("INFO", "Server constructed"); }

bool Server::isRunning() const { return _isRunning; }

void Server::start() {
    _logger->log("INFO", "Server started");
    _isRunning = true;
    _serverfd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sin;
    sin.sin_port = 8080;

    bind(_serverfd, *sin_);
}

void Server::stop() {
    _isRunning = false;
    _logger->log("INFO", "Server stopped");
}
