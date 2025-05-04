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

class Connection {
  private:
    sockaddr_storage _addr;
    std::string _buf;
    int _fd;

  public:
    Connection(struct sockaddr_storage addr, int fd) : _addr(addr), _fd(fd) {}
    void readIntoBuf() {
        char newbuf[1024];
        ssize_t r = recv(_fd, newbuf, 1024, 0);
        newbuf[r] = '\0';
        _buf += newbuf;
    }

    int parseBuf(IHttpParser* prsr) {
        char* b = (char*)_buf.c_str();
        while (*b) {
            prsr->feed(b, 1);
            if (prsr->error() || prsr->ready()) {
                _buf = b + 1;
                return 1;
            }
            b++;
        }
        _buf = b;
        return 0;
    }
    sockaddr_storage getAddr() const { return _addr; }
};

class ConnectionHandler : public IConnectionHandler {
  private:
    std::map<int, Connection*> _connections;
    std::map<int, IHttpParser*> _parsers;
    std::map<int, HttpResponse> _responses;
    ILogger& _logger;
    IIONotifier& _ioNotifier;
    void _addClientConnection(int conn, struct sockaddr_storage theirAddr);
    int _acceptNewConnection(int socketfd);
    void _onSocketRead(int fd, bool withRead);
    void _sendPipeline(int conn);
    void _removeClientConnection(int conn);

  public:
    ConnectionHandler(ILogger&, IIONotifier&);
    ~ConnectionHandler(void);
    int handleConnection(int conn, e_notif notif);
};

#endif // CONNECTIONHANDLER_H
