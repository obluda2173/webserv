#include "EPollManager.h"
#include <string.h>
#include <sys/epoll.h>

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

int EPollManager::wait(int* fd) {
    struct epoll_event events[1]; // TODO: make maxEvents configurable
    int ready = epoll_wait(_epfd, events, 1, 10);
    if (ready > 0)
        *fd = events[0].data.fd;
    else
        *fd = -1;
    return ready;
}
