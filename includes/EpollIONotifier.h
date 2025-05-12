#ifndef EPOLLMANAGER_H
#define EPOLLMANAGER_H

#include <ILogger.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <IIONotifier.h>

#ifndef NBR_EVENTS_NOTIFIER
#define NBR_EVENTS_NOTIFIER 10
#endif

class EpollIONotifier : public IIONotifier {
  private:
    ILogger& _logger;
    int _epfd;

  public:
    EpollIONotifier(ILogger& logger);
    ~EpollIONotifier(void);
    void add(int fd);
    void modify(int fd, e_notif notif);
    void del(int fd);
    int wait(int* fds, e_notif* notif);
};

#endif // EPOLLMANAGER_H
