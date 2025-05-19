#ifndef EPOLLMANAGER_H
#define EPOLLMANAGER_H

#include "IIONotifier.h"
#include "ILogger.h"
#include <ctime>
#include <sys/epoll.h>
#include <sys/time.h>
#include <unistd.h>

#ifndef NBR_EVENTS_NOTIFIER
#define NBR_EVENTS_NOTIFIER 10
#endif

class IClock {
  public:
    virtual ~IClock() {}
    virtual timeval now() const = 0;
};

class SystemClock : public IClock {
  public:
    timeval now() const {
        timeval now_;
        gettimeofday(&now_, NULL);
        return now_;
    }
};

class EpollIONotifier : public IIONotifier {
  private:
    ILogger& _logger;
    IClock* _clock;
    int _epfd;
    unsigned int _timeout_ms;
    timeval _lastTime;
    timeval _now;
    int _fd;

  public:
    EpollIONotifier(ILogger& logger, IClock* clock = new SystemClock());
    ~EpollIONotifier(void);
    void add(int fd, unsigned int timeout_ms = 30000);
    void modify(int fd, e_notif notif);
    void del(int fd);
    std::vector< t_notif > wait(void);
};

#endif // EPOLLMANAGER_H
