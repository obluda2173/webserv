#include "Listener.h"
#include "EPollManager.h"
#include "IConnectionHandler.h"
#include <cstring>
#include <netinet/in.h>
#include <sys/epoll.h>

Listener::Listener(ILogger& logger, IConnectionHandler* connHdlr, EPollManager* epollMngr)
    : _logger(logger), _connHdlr(connHdlr), _epollMngr(epollMngr) {}

Listener::~Listener() {
    for (size_t i = 0; i < _socketfds.size(); i++)
        close(_socketfds[i]);
    delete _connHdlr;
    delete _epollMngr;
}

void Listener::listen() {
    _isListening = true;
    while (_isListening) {
        int fd;
        int ready = _epollMngr->wait(&fd);
        if (ready == 0)
            continue;
        if (!_isListening)
            break;

        _connHdlr->handleConnection(fd);
    }
}

void Listener::stop() { _isListening = false; }

void Listener::add(int socketfd) {
    _epollMngr->add(socketfd, READY_TO_READ);
    _socketfds.push_back(socketfd);
}
