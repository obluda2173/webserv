#include "EPollManager.h"
#include <string.h>

EPollManager::EPollManager(ILogger* logger) : _logger(logger) {
    _epfd = epoll_create(1);
    if (_epfd == -1) {
        _logger->log("ERROR", "epoll_create: " + std::string(strerror(errno)));
        exit(1);
    }
}

EPollManager::~EPollManager(void) {
    if (close(_epfd) == -1) {
        _logger->log("ERROR", "close: " + std::string(strerror(errno)));
        exit(1);
    }
}

void EPollManager::add(int socketfd, uint32_t listenEvent) {
    struct epoll_event event;
    event.events = listenEvent; // Monitor for read events
    event.data.fd = socketfd;
    epoll_ctl(_epfd, EPOLL_CTL_ADD, socketfd, &event);
}

void EPollManager::del(int socketfd) { epoll_ctl(_epfd, EPOLL_CTL_DEL, socketfd, NULL); }

int EPollManager::wait(struct epoll_event* events, int nEvents) { return epoll_wait(_epfd, events, nEvents, 10); }
