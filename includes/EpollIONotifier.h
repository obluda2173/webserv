#ifndef EPOLLMANAGER_H
#define EPOLLMANAGER_H

#include <ILogger.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <IIONotifier.h>

// #define CLIENT_HUNG_UP EPOLLRDHUP
// #define READY_TO_READ EPOLLIN

class EpollIONotifier : public IIONotifier {
  private:
    ILogger& _logger;
    int _epfd;

  public:
    EpollIONotifier(ILogger& logger);
    ~EpollIONotifier(void);
    void add(int fd, e_notif notif);
    void modify(int fd, e_notif notif);
    void del(int fd);
    int wait(int* fds, e_notif* notif);
};

#endif // EPOLLMANAGER_H
