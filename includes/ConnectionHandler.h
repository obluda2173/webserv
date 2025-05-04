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
  public:
    enum STATE { ReadingHeaders, WritingResponse, WritingError };

  private:
    STATE _state;
    sockaddr_storage _addr;
    std::string _buf;
    int _fd;
    IHttpParser* _prsr;

  public:
    ~Connection() {
        close(_fd);
        delete _prsr;
    }
    Connection(sockaddr_storage addr, int fd, IHttpParser* prsr) : _addr(addr), _fd(fd), _prsr(prsr) {
        _state = ReadingHeaders;
    }
    STATE getState() const { return _state; }
    void readIntoBuf() {
        char newbuf[1024];
        ssize_t r = recv(_fd, newbuf, 1024, 0);
        newbuf[r] = '\0';
        _buf += newbuf;
    }

    void parseBuf() {
        if (_prsr->error() || _prsr->ready())
            _prsr->resetPublic();
        char* b = (char*)_buf.c_str();
        while (*b) {
            _prsr->feed(b, 1);
            if (_prsr->error() || _prsr->ready()) {
                _buf = b + 1;
                if (_prsr->error()) {
                    _state = WritingError;
                } else {
                    _state = WritingResponse;
                }
                return;
            }
            b++;
        }
        _buf = b;
        _state = ReadingHeaders;
    }
    sockaddr_storage getAddr() const { return _addr; }
};

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

    void _updateNotifier(int connfd);

  public:
    ConnectionHandler(ILogger&, IIONotifier&);
    ~ConnectionHandler(void);
    int handleConnection(int conn, e_notif notif);
};

#endif // CONNECTIONHANDLER_H
