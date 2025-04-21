#include "Server.h"
#include "IIONotifier.h"
#include "ILogger.h"
#include "Listener.h"
#include "utils.h"
#include <cstddef>

Server::Server(ILogger* logger, IConnectionHandler* connHdlr, IIONotifier* _ioNotif)
    : _logger(logger), _listener(new Listener(*logger, connHdlr, _ioNotif)) {}

Server::~Server() {
    delete _logger;
    delete _listener;
}

bool Server::isRunning() const { return _isRunning; }

void Server::start(std::vector<std::string> ports) {
    _logger->log("INFO", "Server is starting...");

    for (size_t i = 0; i < ports.size(); i++)
        _listener->add(newListeningSocket(NULL, ports[i].c_str()));

    _isRunning = true;
    _logger->log("INFO", "Server started");
    _listener->listen();
}

void Server::stop() {
    _logger->log("INFO", "Server is stopping...");
    _listener->stop();
    _isRunning = false;
    _logger->log("INFO", "Server stopped");
}
