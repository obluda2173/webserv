#ifndef LISTENER_H
#define LISTENER_H

#include "EPollManager.h"
#include "IListener.h"
#include "ILogger.h"
#include "utils.h"
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
#include <vector>

class Listener : public IListener {
  private:
    ILogger* _logger;
    EPollManager* _epollMngr;
    std::vector<int> _portfds;
    std::vector<ConnectionInfo*> _portfds_infos;
    bool _isListening;
    void _addClientSocket(int clientSocket, ConnectionInfo* connInfo);

  public:
    Listener(ILogger* logger, EPollManager* epollMngr);
    ~Listener();
    void listen();
    void stop();
    void add(int socketfd);
};

#endif // LISTENER_H
