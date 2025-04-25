#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include "IConnectionHandler.h"
#include "IHttpParser.h"
#include "IIONotifier.h"
#include "ILogger.h"
#include <map>
#include <sys/socket.h>

typedef enum SocketType {
    PORT_SOCKET,
    CLIENT_SOCKET,
} SocketType;

struct ConnectionInfo {
    struct sockaddr_storage addr;
    int fd;
};

class ConnectionHandler : public IConnectionHandler {
  private:
    std::map<int, ConnectionInfo> _connections;
    std::map<int, IHttpParser*> _parsers;
    std::map<int, std::string> _responses;
    ILogger& _logger;
    IIONotifier& _ioNotifier;
    void _addClientConnection(int conn, struct sockaddr_storage theirAddr);
    void _removeClientConnection(ConnectionInfo connInfo);
    int _acceptNewConnection(int socketfd);
    void _readPipeline(int conn);
    void _sendPipeline(int conn);

  public:
    ConnectionHandler(ILogger&, IIONotifier&);
    ~ConnectionHandler(void);
    int handleConnection(int conn, e_notif notif);
};

#endif // CONNECTIONHANDLER_H
