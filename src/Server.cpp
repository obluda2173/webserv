#include "Server.h"
#include "ILogger.h"
#include "Listener.h"
#include "utils.h"
#include <cstddef>
#include <errno.h>

Server::Server(ILogger* logger, IConnectionHandler* connHdlr, EPollManager* epollMngr)
    : _logger(logger), _listener(new Listener(logger, connHdlr, epollMngr)), _epollMngr(epollMngr) {}

Server::~Server() { delete _listener; }

bool Server::isRunning() const { return _isRunning; }

void Server::start(std::vector<int> ports) {
    _logger->log("INFO", "Server is starting...");

    for (size_t i = 0; i < ports.size(); i++)
        _portfds.push_back(new_socket(ports[i]));

    for (size_t i = 0; i < _portfds.size(); i++)
        _listener->add(_portfds[i]);

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
