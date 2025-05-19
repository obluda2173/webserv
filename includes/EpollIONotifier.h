#ifndef EPOLLMANAGER_H
#define EPOLLMANAGER_H

#include "IClock.h"
#include "IIONotifier.h"
#include "ILogger.h"
#include "SystemClock.h"
#include <ctime>
#include <sys/epoll.h>
#include <sys/time.h>
#include <unistd.h>

#ifndef NBR_EVENTS_NOTIFIER
#define NBR_EVENTS_NOTIFIER 10
#endif

class EpollIONotifier : public IIONotifier {
  private:
    ILogger& _logger;
    IClock* _clock;
    int _epfd;
    long _timeout_ms;
    timeval _lastTime;
    timeval _now;
    int _fd;

  public:
    EpollIONotifier(ILogger& logger, IClock* clock = new SystemClock());
    ~EpollIONotifier(void);
    void add(int fd, long timeout_ms = 30000);
    void modify(int fd, e_notif notif);
    void del(int fd);
    std::vector< t_notif > wait(void);
};

#endif // EPOLLMANAGER_H
