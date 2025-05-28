#ifndef ILISTENER_H
#define ILISTENER_H

#include <csignal>

class IListener {
  public:
    virtual ~IListener() {};
    virtual void listen(volatile sig_atomic_t* running) = 0;
    virtual void stop() = 0;
    virtual void add(int socketfd) = 0;
};

#endif // ILISTENER_H
