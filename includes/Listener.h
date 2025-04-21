#ifndef LISTENER_H
#define LISTENER_H

#include "IConnectionHandler.h"
#include "IIONotifier.h"
#include "IListener.h"
#include "ILogger.h"
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
    ILogger& _logger;
    IConnectionHandler* _connHdlr;
    IIONotifier* _ioNotifier;
    std::vector<int> _socketfds;
    bool _isListening;

  public:
    Listener(ILogger&, IConnectionHandler*, IIONotifier*);
    ~Listener();
    void listen();
    void processEvents();
    void stop();
    void add(int socketfd);
};

#endif // LISTENER_H
