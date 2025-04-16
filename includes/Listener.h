#ifndef LISTENER_H
#define LISTENER_H

#include "ILogger.h"

class Listener {
  private:
    int _portfd;
    bool _isListening;
    ILogger *_logger;

  public:
    Listener();
    Listener(int &portfd, ILogger *logger);
    void listen();
    void stop();
};

#endif // LISTENER_H
