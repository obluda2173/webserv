#include "IConnectionHandler.h"
#include "IIONotifier.h"
#include "IListener.h"
#include "ILogger.h"
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <set>
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
    bool _isRunning;
    std::vector< int > _portfds;

  public:
    explicit Server(ILogger* logger, IConnectionHandler* connHdlr, IIONotifier* _ioNotifier);
    ~Server();
    bool isRunning() const;
    void start(std::set< std::pair< std::string, std::string > > addrPorts);
    void stop();
};
