#include "EpollIONotifier.h"
#include "IIONotifier.h"
#include <ctime>
#include <errno.h>
#include <iostream>
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

void EpollIONotifier::add(int fd, unsigned int timeout_ms) {
    _timeout_ms = timeout_ms;
    _lastTime = _clock->now();
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

double diffTime(timeval start, timeval end) {
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    double elapsedMillis = seconds * 1000.0 + microseconds / 1000.0;
    return elapsedMillis;
}

void printTimeval(const timeval& tv) {
    std::cout << "Seconds: " << tv.tv_sec << ", Microseconds: " << tv.tv_usec << std::endl;
}

std::vector< t_notif > EpollIONotifier::wait(void) {
    std::vector< t_notif > results;
    struct epoll_event events[NBR_EVENTS_NOTIFIER];
    int ready = epoll_wait(_epfd, events, NBR_EVENTS_NOTIFIER, 10);

    _now = _clock->now();

    if (_timeout_ms < diffTime(_lastTime, _now)) {
        results.push_back(t_notif{-1, READY_TO_READ});
    }
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
