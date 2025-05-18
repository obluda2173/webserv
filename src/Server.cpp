#include "Server.h"
#include "IIONotifier.h"
#include "ILogger.h"
#include "Listener.h"
#include "utils.h"
#include <cstddef>

Server::Server(ILogger* logger, IConnectionHandler* connHdlr, IIONotifier* _ioNotifier)
    : _logger(logger), _listener(new Listener(*logger, connHdlr, _ioNotifier)) {}

Server::~Server() {
    delete _logger;
    delete _listener;
}

bool Server::isRunning() const { return _isRunning; }

void Server::start(std::vector< std::string > ports) {
    _logger->log("INFO", "Server is starting...");

    for (size_t i = 0; i < ports.size(); i++) {
        struct addrinfo* svrAddrInfo;
        getAddrInfoHelper(NULL, ports[i].c_str(), AF_INET, &svrAddrInfo);
        int serverfd = newListeningSocket(svrAddrInfo, 5);
        freeaddrinfo(svrAddrInfo);
        _listener->add(serverfd);
    }

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
