#ifndef LISTENER_H
#define LISTENER_H

#include "IListener.h"
#include "ILogger.h"

class Listener : public IListener {
  private:
    int _socketfd;
    bool _isListening;
    ILogger* _logger;

  public:
    Listener(ILogger* logger);
    ~Listener();
    void listen();
    void stop();
    void add(int socketfd);
};

#endif // LISTENER_H
