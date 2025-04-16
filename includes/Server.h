#include "ILogger.h"
#include "Listener.h"

class Server {
  private:
    ILogger *_logger;
    bool _isRunning;
    int _serverfd;
    Listener _listener;
    void _listenPoll(void);
    void _listenEPoll(void);

  public:
    explicit Server(ILogger *l);
    bool isRunning() const;
    void start();
    void stop();
};
