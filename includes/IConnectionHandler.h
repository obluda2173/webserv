#ifndef ICONNECTIONHANDLER_H
#define ICONNECTIONHANDLER_H

#include "IIONotifier.h"

class IConnectionHandler {
  public:
    virtual ~IConnectionHandler(void) {}
    virtual int handleConnection(int conn, e_notif notif) = 0;
};

#endif // ICONNECTIONHANDLER_H
