#ifndef LISTENER_H
#define LISTENER_H

#include "IListener.h"
#include "ILogger.h"
#include <vector>

class Listener : public IListener {
  private:
    int _socketfd;
    bool _isListening;
    ILogger* _logger;
    int _epfd;

  public:
    Listener(ILogger* logger);
    ~Listener();
    void listen();
    void stop();
    void add(int socketfd);
    void add(std::vector<int> socketfd);
};

#endif // LISTENER_H
