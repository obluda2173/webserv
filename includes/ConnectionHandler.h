#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include "EPollManager.h"
#include "IConnectionHandler.h"
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
    EPollManager& _epollMngr;
    void _addClientConnection(int conn, struct sockaddr* theirAddr);
    void _removeClientConnection(ConnectionInfo connInfo);
    void _acceptNewConnection(int socketfd);

  public:
    ConnectionHandler(ILogger&, EPollManager&);
    ~ConnectionHandler(void);
    void handleConnection(int conn);
};

#endif // CONNECTIONHANDLER_H
