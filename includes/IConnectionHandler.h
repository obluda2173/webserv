#ifndef ICONNECTIONHANDLER_H
#define ICONNECTIONHANDLER_H

#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

typedef enum SocketType {
    PORT_SOCKET,
    CLIENT_SOCKET,
} SocketType;

struct ConnectionInfo {
    struct sockaddr_in addr;
    SocketType type;
    int fd;
};

class IConnectionHandler {
  public:
    virtual ~IConnectionHandler(void) {}
    virtual void handleConnection(ConnectionInfo* connInfo) = 0;
};

#endif // ICONNECTIONHANDLER_H
