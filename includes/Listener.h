#ifndef LISTENER_H
#define LISTENER_H

#include "IListener.h"
#include "ILogger.h"
#include <vector>

#include <arpa/inet.h>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class Listener : public IListener {
  private:
    std::vector<int> _portfds;
    bool _isListening;
    ILogger* _logger;
    int _epfd;

  public:
    Listener(ILogger* logger);
    ~Listener();
    void listen();
    void stop();
    void add(int socketfd);
};

#endif // LISTENER_H
