#include "EPollManager.h"
#include "IConnectionHandler.h"
#include "IListener.h"
#include "ILogger.h"
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

class Server {
  private:
    ILogger* _logger;
    IListener* _listener;
    IConnectionHandler* _connHdlr;
    EPollManager* _epollMngr;
    bool _isRunning;
    std::vector<int> _portfds;
    void _listenPoll(void);
    void _listenEPoll(void);

  public:
    explicit Server(ILogger* logger, IConnectionHandler* connHdlr, EPollManager* _epollMngr);
    ~Server();
    bool isRunning() const;
    void start(std::vector<std::string> ports);
    void stop();
};
