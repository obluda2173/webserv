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
    std::map< int, Connection* > _connections;
    IRouter* _router;
    std::map< int, IRouter* > _routers;
    ILogger& _logger;
    IIONotifier& _ioNotifier;
    void _addClientConnection(int connfd, struct sockaddr_storage theirAddr, int port);
    int _acceptNewConnection(int socketfd);
    void _onSocketRead(Connection* conn);
    void _onSocketWrite(Connection* conn);
    void _handleState(Connection* conn);
    void _updateNotifier(Connection* conn);
    void _removeConnection(int connfd);

  public:
    ConnectionHandler(std::map< int, IRouter* >, ILogger&, IIONotifier&);
    ~ConnectionHandler(void);
    int handleConnection(int conn, e_notif notif);
};

#endif // CONNECTIONHANDLER_H
