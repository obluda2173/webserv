#include "Listener.h"
#include "EPollManager.h"
#include "IConnectionHandler.h"
#include <cstring>
#include <netinet/in.h>
#include <sys/epoll.h>

Listener::Listener(ILogger* logger, IConnectionHandler* connHdlr, EPollManager* epollMngr)
    : _logger(logger), _connHdlr(connHdlr), _epollMngr(epollMngr) {}

Listener::~Listener() {}

void Listener::listen() {
    struct epoll_event events[1];

    _isListening = true;
    while (_isListening) {
        int ready = _epollMngr->wait(events, 1);
        if (ready == 0)
            continue;
        if (!_isListening)
            break;
        _connHdlr->handleConnection((ConnectionInfo*)events[0].data.ptr);
    }
}

void Listener::stop() {
    _isListening = false;
    for (size_t i = 0; i < _portfds_infos.size(); i++)
        delete _portfds_infos[i];
}

void Listener::add(int portfd) {
    ConnectionInfo* connInfo = new ConnectionInfo;
    connInfo->fd = portfd;
    connInfo->type = PORT_SOCKET;

    _epollMngr->add(portfd, connInfo, READY_TO_READ);
    _portfds.push_back(portfd);
    _portfds_infos.push_back(connInfo);
}
