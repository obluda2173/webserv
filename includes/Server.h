#include "IListener.h"
#include "ILogger.h"
#include <vector>

class Server {
  private:
    ILogger* _logger;
    IListener* _listener;
    bool _isRunning;
    std::vector<int> _portfds;
    void _listenPoll(void);
    void _listenEPoll(void);

  public:
    explicit Server(ILogger* li);
    ~Server();
    bool isRunning() const;
    void start(std::vector<int> ports);
    void stop();
};
