#ifndef ICONNECTIONHANDLER_H
#define ICONNECTIONHANDLER_H

#include "utils.h"

class IConnectionHandler {
  public:
    virtual ~IConnectionHandler(void) {}
    virtual void handleConnection(ConnectionInfo* connInfo) = 0;
};

#endif // ICONNECTIONHANDLER_H
