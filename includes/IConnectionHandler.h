#ifndef ICONNECTIONHANDLER_H
#define ICONNECTIONHANDLER_H

#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class IConnectionHandler {
  public:
    virtual ~IConnectionHandler(void) {}
    virtual void handleConnection(int conn) = 0;
};

#endif // ICONNECTIONHANDLER_H
