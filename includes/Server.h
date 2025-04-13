#include "ILogger.h"

class Server {
  private:
    ILogger *_logger;
    bool _isRunning;
    int _serverfd;

  public:
    explicit Server(ILogger *l);
    bool isRunning() const;
    void start();
    void stop();
};
