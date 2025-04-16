#ifndef ILISTENER_H
#define ILISTENER_H

class IListener {
  public:
    virtual ~IListener() {};
    virtual void listen() = 0;
    virtual void stop() = 0;
    virtual void add(int socketfd) = 0;
};

#endif // ILISTENER_H
