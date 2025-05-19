#include "Listener.h"
#include "EpollIONotifier.h"
#include "IConnectionHandler.h"
#include "IIONotifier.h"
#include <cstring>
#include <netinet/in.h>
#include <sys/epoll.h>

Listener::Listener(ILogger& logger, IConnectionHandler* connHdlr, IIONotifier* epollMngr)
    : _logger(logger), _connHdlr(connHdlr), _ioNotifier(epollMngr) {}

Listener::~Listener() {
    delete _connHdlr;
    delete _ioNotifier;
}

void Listener::listen() {
    _isListening = true;
    while (_isListening) {
        std::vector< t_notif > notifs;
        notifs = _ioNotifier->waitVector();
        if (notifs.size() == 0)
            continue;
        if (!_isListening)
            break;

        for (size_t i = 0; i < notifs.size(); i++)
            _connHdlr->handleConnection(notifs[i].fd, notifs[i].notif);
    }
}

void Listener::stop() {
    _isListening = false;
    for (size_t i = 0; i < _socketfds.size(); i++)
        close(_socketfds[i]);
}

void Listener::add(int socketfd) {
    _ioNotifier->add(socketfd);
    _socketfds.push_back(socketfd);
}
