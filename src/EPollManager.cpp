#include "EpollIONotifier.h"
#include "IIONotifier.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

EpollIONotifier::EpollIONotifier(ILogger& logger) : _logger(logger) {
    _epfd = epoll_create(1);
    if (_epfd == -1) {
        _logger.log("ERROR", "epoll_create: " + std::string(strerror(errno)));
        exit(1);
    }
}

EpollIONotifier::~EpollIONotifier(void) {
    if (close(_epfd) == -1) {
        _logger.log("ERROR", "close: " + std::string(strerror(errno)));
        exit(1);
    }
}

void EpollIONotifier::add(int socketfd, e_notif notif) {
    struct epoll_event event;
    if (notif == CLIENT_HUNG_UP)
        event.events = EPOLLRDHUP;
    if (notif == READY_TO_READ)
        event.events = EPOLLIN;
    event.data.fd = socketfd;
    epoll_ctl(_epfd, EPOLL_CTL_ADD, socketfd, &event);
}

void EpollIONotifier::del(int socketfd) { epoll_ctl(_epfd, EPOLL_CTL_DEL, socketfd, NULL); }

int EpollIONotifier::wait(int* fd) {
    struct epoll_event events[1]; // TODO: make maxEvents configurable
    int ready = epoll_wait(_epfd, events, 1, 10);
    if (ready > 0)
        *fd = events[0].data.fd;
    else
        *fd = -1;
    return ready;
}
