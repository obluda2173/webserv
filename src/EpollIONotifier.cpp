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

void EpollIONotifier::add(int fd) {
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLRDHUP;
    event.data.fd = fd;
    epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &event);
}

void EpollIONotifier::del(int fd) { epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL); }

void EpollIONotifier::modify(int fd, e_notif notif) {
    struct epoll_event event;
    if (notif == READY_TO_WRITE) {
        event.events = EPOLLOUT;
    } else if (notif == READY_TO_READ) {
        event.events = EPOLLIN | EPOLLRDHUP;
    }
    event.data.fd = fd;
    epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &event);
}

int EpollIONotifier::wait(int* fds, e_notif* notifs) {
    struct epoll_event events[NBR_EVENTS_NOTIFIER];
    int ready = epoll_wait(_epfd, events, NBR_EVENTS_NOTIFIER, 10);

    if (ready > 0) {
        for (int i = 0; i < ready; i++) {
            fds[i] = events[i].data.fd;
            if (events[i].events & EPOLLHUP)
                notifs[i] = BROKEN_CONNECTION;
            else if (events[i].events & EPOLLRDHUP) {
                notifs[i] = CLIENT_HUNG_UP;
            } else if (events[i].events & EPOLLIN)
                notifs[i] = READY_TO_READ;
            else if (events[i].events & EPOLLOUT)
                notifs[i] = READY_TO_WRITE;
        }
    } else
        *fds = -1;
    return ready;
}

std::vector< t_notif > EpollIONotifier::waitVector(void) {
    std::vector< t_notif > results;
    struct epoll_event events[NBR_EVENTS_NOTIFIER];
    int ready = epoll_wait(_epfd, events, NBR_EVENTS_NOTIFIER, 10);

    if (ready > 0) {
        for (int i = 0; i < ready; i++) {
            int fd = events[i].data.fd;
            if (events[i].events & EPOLLHUP)
                results.push_back(t_notif{fd, BROKEN_CONNECTION});
            else if (events[i].events & EPOLLRDHUP)
                results.push_back(t_notif{fd, CLIENT_HUNG_UP});
            else if (events[i].events & EPOLLIN)
                results.push_back(t_notif{fd, READY_TO_READ});
            else if (events[i].events & EPOLLOUT)
                results.push_back(t_notif{fd, READY_TO_WRITE});
        }
    }
    return results;
}
