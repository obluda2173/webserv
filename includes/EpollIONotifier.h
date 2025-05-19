#ifndef EPOLLMANAGER_H
#define EPOLLMANAGER_H

#include "IIONotifier.h"
#include "ILogger.h"
#include <ctime>
#include <sys/epoll.h>
#include <unistd.h>

#ifndef NBR_EVENTS_NOTIFIER
#define NBR_EVENTS_NOTIFIER 10
#endif

class EpollIONotifier : public IIONotifier {
  private:
    ILogger& _logger;
    int _epfd;
    int _timeout_ms;
    timeval _lastTime;
    timeval _now;

  public:
    EpollIONotifier(ILogger& logger);
    ~EpollIONotifier(void);
    void add(int fd, unsigned int timeout_ms = 30000);
    void modify(int fd, e_notif notif);
    void del(int fd);
    std::vector< t_notif > wait(void);
};

#endif // EPOLLMANAGER_H
