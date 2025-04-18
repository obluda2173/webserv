#ifndef EPOLLMANAGER_H
#define EPOLLMANAGER_H

#include <ILogger.h>
#include <cstdint>
#include <sys/epoll.h>
#include <unistd.h>

#define CLIENT_HUNG_UP EPOLLRDHUP
#define READY_TO_READ EPOLLIN

#include "IConnectionHandler.h"

class EPollManager {
  private:
    ILogger* _logger;
    int _epfd;

  public:
    EPollManager(ILogger* logger);
    ~EPollManager(void);
    void add(int socketfd, ConnectionInfo* connInfo, uint32_t listenEvent);
    void del(int socketfd);
    int wait(struct epoll_event* events, int nEvents);
};

#endif // EPOLLMANAGER_H
