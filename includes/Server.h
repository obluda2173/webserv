#include "IListener.h"
#include "ILogger.h"

class Server {
  private:
    ILogger* _logger;
    IListener* _listener;
    bool _isRunning;
    int _serverfd;
    void _listenPoll(void);
    void _listenEPoll(void);

  public:
    explicit Server(ILogger* li);
    ~Server();
    bool isRunning() const;
    void start();
    void stop();
};
