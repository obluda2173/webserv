#include "Server.h"
#include "IIONotifier.h"
#include "ILogger.h"
#include "Listener.h"
#include "utils.h"
#include <set>

Server::Server(ILogger* logger, IConnectionHandler* connHdlr, IIONotifier* _ioNotifier)
    : _logger(logger), _listener(new Listener(*logger, connHdlr, _ioNotifier)) {}

Server::~Server() {
    delete _logger;
    delete _listener;
}

bool Server::isRunning() const { return _isRunning; }

void Server::start(std::set< std::pair< std::string, std::string > > addrPorts) {
    _logger->log("INFO", "Server is starting...");

    for (std::set< std::pair< std::string, std::string > >::iterator it = addrPorts.begin(); it != addrPorts.end();
         it++) {
        struct addrinfo* svrAddrInfo;
        getAddrInfoHelper(it->first.c_str(), it->second.c_str(), AF_INET, &svrAddrInfo);
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
