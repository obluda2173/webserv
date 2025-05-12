#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include "Connection.h"
#include "IConnectionHandler.h"
#include "IIONotifier.h"
#include "ILogger.h"
#include "IRouter.h"
#include <map>
#include <sys/socket.h>

typedef enum SocketType {
    PORT_SOCKET,
    CLIENT_SOCKET,
} SocketType;

class ConnectionHandler : public IConnectionHandler {
  private:
    std::map<int, Connection*> _connections;
    IRouter* _router;
    ILogger& _logger;
    IIONotifier& _ioNotifier;
    void _addClientConnection(int conn, struct sockaddr_storage theirAddr);
    int _acceptNewConnection(int socketfd);
    void _onSocketRead(int fd);
    void _onSocketWrite(int conn);
    void _handleState(Connection* conn);
    void _onClientHungUp(int conn);
    void _updateNotifier(Connection* conn);
    void _removeConnection(int connfd);

  public:
    ConnectionHandler(IRouter*, ILogger&, IIONotifier&);
    ~ConnectionHandler(void);
    int handleConnection(int conn, e_notif notif);
};

#endif // CONNECTIONHANDLER_H
