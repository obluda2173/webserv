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

struct ConnectionInfo {
    struct sockaddr_in addr;
    int fd;
};

class Listener : public IListener {
  private:
    std::vector<int> _portfds;
    std::vector<int> _activeConns;
    std::vector<ConnectionInfo*> _portfds_infos;
    int _epfd;
    ILogger* _logger;
    bool _isListening;

  public:
    Listener(ILogger* logger);
    ~Listener();
    void listen();
    void stop();
    void add(int socketfd);
};

#endif // LISTENER_H
