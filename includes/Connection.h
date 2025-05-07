#ifndef CONNECTION_H
#define CONNECTION_H

#include "HttpResponse.h"
#include "IHttpParser.h"
#include "ResponseWriter.h"
#include <string>
#include <sys/socket.h>
#include <unistd.h>

class Connection {
  public:
    enum STATE { ReadingHeaders, Handling, HandleBadRequest, SendResponse };

  private:
    STATE _state;
    sockaddr_storage _addr;
    std::string _buf;
    int _fd;
    IHttpParser* _prsr;
    IResponseWriter* _wrtr;

  public:
    ~Connection();
    Connection(sockaddr_storage addr, int fd, IHttpParser* prsr);
    STATE getState() const;
    void readIntoBuf();
    void parseBuf();
    void sendResponse();
    void resetResponse();
    int getFileDes() const;
    HttpRequest getRequest();
    HttpRequest _request;
    sockaddr_storage getAddr() const;
    void setState(Connection::STATE state) { _state = state; }

    HttpResponse _response;
};

#endif // CONNECTION_H
