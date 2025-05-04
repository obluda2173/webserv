#ifndef CONNECTION_H
#define CONNECTION_H

#include "IHttpParser.h"
#include <string>
#include <sys/socket.h>
#include <unistd.h>

class Connection {
  public:
    enum STATE { ReadingHeaders, WritingResponse, WritingError, SendResponse };

  private:
    STATE _state;
    sockaddr_storage _addr;
    std::string _buf;
    int _fd;
    IHttpParser* _prsr;

  public:
    ~Connection();
    Connection(sockaddr_storage addr, int fd, IHttpParser* prsr);
    STATE getState() const;
    void readIntoBuf();
    void parseBuf();
    int getFileDes() const;
    sockaddr_storage getAddr() const;
    std::string _response;
    int _statusCode;
    void setStateToSendResponse() { _state = SendResponse; }
};

#endif // CONNECTION_H
