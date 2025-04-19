#include "Listener.h"
#include "EPollManager.h"
#include "IConnectionHandler.h"
#include <cstring>
#include <netinet/in.h>
#include <sys/epoll.h>

Listener::Listener(ILogger* logger, IConnectionHandler* connHdlr, EPollManager* epollMngr)
    : _logger(logger), _connHdlr(connHdlr), _epollMngr(epollMngr) {}

Listener::~Listener() {
    for (size_t i = 0; i < _socketfds.size(); i++)
        close(_socketfds[i]);
}

void Listener::listen() {
    struct epoll_event events[1];

    _isListening = true;
    while (_isListening) {
        int ready = _epollMngr->wait(events, 1);
        if (ready == 0) {
            continue;
        }
        if (!_isListening) {
            break;
        }

        // TODO: Make test for Reading something: only the event should probably be sent:
        _connHdlr->handleConnection(events[0].data.fd);
    }
}

void Listener::stop() { _isListening = false; }

void Listener::add(int socketfd) {
    _epollMngr->add(socketfd, READY_TO_READ);
    _socketfds.push_back(socketfd);
}
