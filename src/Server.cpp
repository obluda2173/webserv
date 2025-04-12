#include "Server.h"

Server::Server(ILogger *l) : _logger(l) {
    _logger->log("INFO", "Server constructed");
}

bool Server::isRunning() const {
    return _isRunning;
}

void Server::start() {
	_isRunning = true;
}
