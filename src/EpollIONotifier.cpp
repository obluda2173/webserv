#include "EpollIONotifier.h"
#include "IIONotifier.h"
#include <ctime>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <unistd.h>

EpollIONotifier::EpollIONotifier(ILogger& logger, IClock* clock) : _logger(logger), _clock(clock) {
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
    delete _clock;
}

void EpollIONotifier::add(int fd, long timeout_ms) {
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLRDHUP;
    event.data.fd = fd;
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &event);
    _fdInfos[fd].lastActivity = _clock->now();
    _fdInfos[fd].timeout_ms = timeout_ms;
}

void EpollIONotifier::addNoTimeout(int fd) {
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLRDHUP;
    event.data.fd = fd;

    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &event);
}

void EpollIONotifier::del(int fd) {
    _fdInfos.erase(fd);
    epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL);
}

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

double diffTime(timeval start, timeval end) {
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    double elapsedMillis = seconds * 1000.0 + microseconds / 1000.0;
    return elapsedMillis;
}

std::vector< t_notif > EpollIONotifier::wait(void) {
    std::vector< t_notif > results;
    struct epoll_event events[NBR_EVENTS_NOTIFIER];
    int ready = epoll_wait(_epfd, events, NBR_EVENTS_NOTIFIER, 10);

    _now = _clock->now();
    if (ready > 0) {
        for (int i = 0; i < ready; i++) {
            int fd = events[i].data.fd;
            t_notif notif;
            notif.fd = fd;

            if (events[i].events & EPOLLHUP)
                notif.notif = BROKEN_CONNECTION;
            else if (events[i].events & EPOLLRDHUP)
                notif.notif = CLIENT_HUNG_UP;
            else if (events[i].events & EPOLLIN)
                notif.notif = READY_TO_READ;
            else if (events[i].events & EPOLLOUT)
                notif.notif = READY_TO_WRITE;

            if (notif.notif != READY_TO_WRITE && _fdInfos.find(fd) != _fdInfos.end())
                _fdInfos[fd].lastActivity = _now;

            results.push_back(notif);
        }
    }

    std::map< int, FdInfo >::iterator it = _fdInfos.begin();
    while (it != _fdInfos.end()) {
        std::map< int, FdInfo >::iterator current = it++; // Save current and increment

        if (current->second.timeout_ms <= diffTime(current->second.lastActivity, _now)) {
            t_notif notif;
            notif.fd = current->first;
            notif.notif = TIMEOUT;
            results.push_back(notif);

            _fdInfos.erase(current); // Erase using saved iterator
        }
    }

    return results;
}
