#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include "IConnectionHandler.h"
#include "IIONotifier.h"
#include "ILogger.h"
#include <map>

typedef enum SocketType {
    PORT_SOCKET,
    CLIENT_SOCKET,
} SocketType;

struct ConnectionInfo {
    struct sockaddr_in addr;
    SocketType type;
    int fd;
};

class ConnectionHandler : public IConnectionHandler {
  private:
    std::map<int, ConnectionInfo> _connections;
    ILogger& _logger;
    IIONotifier& _ioNotifier;
    void _addClientConnection(int conn, struct sockaddr* theirAddr);
    void _removeClientConnection(ConnectionInfo connInfo);
    void _acceptNewConnection(int socketfd);

  public:
    ConnectionHandler(ILogger&, IIONotifier&);
    ~ConnectionHandler(void);
    void handleConnection(int conn);
};

#endif // CONNECTIONHANDLER_H
