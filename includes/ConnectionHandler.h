#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include "HttpResponse.h"
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
    std::string buf;
    int fd;
};

class ConnectionHandler : public IConnectionHandler {
  private:
    std::map<int, ConnectionInfo> _connections;
    std::map<int, IHttpParser*> _parsers;
    std::map<int, HttpResponse> _responses;
    ILogger& _logger;
    IIONotifier& _ioNotifier;
    void _addClientConnection(int conn, struct sockaddr_storage theirAddr);
    int _acceptNewConnection(int socketfd);
    void _readPipeline(int conn, bool withRead);
    void _readFromConn(ConnectionInfo* connInfo);
    void _sendPipeline(int conn);
    void _removeClientConnection(int conn);

  public:
    ConnectionHandler(ILogger&, IIONotifier&);
    ~ConnectionHandler(void);
    int handleConnection(int conn, e_notif notif);
};

#endif // CONNECTIONHANDLER_H
