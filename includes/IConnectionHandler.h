#ifndef ICONNECTIONHANDLER_H
#define ICONNECTIONHANDLER_H

#include "IIONotifier.h"
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class IConnectionHandler {
  public:
    virtual ~IConnectionHandler(void) {}
    virtual int handleConnection(int conn, e_notif notif) = 0;
};

#endif // ICONNECTIONHANDLER_H
