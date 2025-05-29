#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include "BodyParser.h"
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
    std::map< std::string, size_t > _sessionIdsDataBase;
    std::map< int, Connection* > _connections;
    std::map< std::string, IRouter* > _routers;
    ILogger& _logger;
    IIONotifier& _ioNotifier;
    BodyParser* _bodyPrsr;
    void _addClientConnection(int connfd, struct sockaddr_storage theirAddr, std::string addrPort);
    int _acceptNewConnection(int socketfd);
    void _onSocketRead(Connection* conn);
    void _onSocketWrite(Connection* conn);
    void _handleState(Connection* conn);
    void _updateNotifier(Connection* conn);
    void _removeConnection(int connfd);

  public:
    ConnectionHandler(std::map< std::string, IRouter* >, ILogger&, IIONotifier&);
    ~ConnectionHandler(void);
    int handleConnection(int conn, e_notif notif);
};

#endif // CONNECTIONHANDLER_H
