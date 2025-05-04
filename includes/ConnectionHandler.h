#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include "Connection.h"
#include "HttpResponse.h"
#include "IConnectionHandler.h"
#include "IIONotifier.h"
#include "ILogger.h"
#include <map>
#include <sys/socket.h>

typedef enum SocketType {
    PORT_SOCKET,
    CLIENT_SOCKET,
} SocketType;

class ConnectionHandler : public IConnectionHandler {
  private:
    std::map<int, Connection*> _connections;
    std::map<int, HttpResponse> _responses;
    ILogger& _logger;
    IIONotifier& _ioNotifier;
    void _addClientConnection(int conn, struct sockaddr_storage theirAddr);
    int _acceptNewConnection(int socketfd);
    void _onSocketRead(int fd);
    void _onSocketWrite(int conn);
    void _onClientHungUp(int conn);
    void _updateNotifier(Connection* conn);

  public:
    ConnectionHandler(ILogger&, IIONotifier&);
    ~ConnectionHandler(void);
    int handleConnection(int conn, e_notif notif);
};

#endif // CONNECTIONHANDLER_H
