#ifndef CONNECTION_H
#define CONNECTION_H

#include "HttpResponse.h"
#include "IHttpParser.h"
#include "ResponseWriter.h"
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#define READ_SIZE 2048

class ISender {
  public:
    virtual ~ISender() {}
    virtual size_t _send(int fd, char* buf, size_t size) = 0;
};

class SystemSender : public ISender {
    virtual size_t _send(int fd, char* buf, size_t n) { return send(fd, buf, n, 0); }
};

class Connection {
  public:
    enum STATE { ReadingHeaders, Handling, HandleBadRequest, SendResponse, Reset };

  private:
    STATE _state;
    sockaddr_storage _addr;
    std::string _readBuf;
    int _fd;
    IHttpParser* _prsr;
    IResponseWriter* _wrtr;
    size_t _readSize;
    ISender* _sender;
    std::vector<char> _sendBuf;
    size_t _sendBufUsedSize;

  public:
    ~Connection();
    Connection(sockaddr_storage addr, int fd, IHttpParser* prsr, size_t readSize, ISender* = new SystemSender());
    STATE getState() const;
    void readIntoBuf();
    void parseBuf();
    void sendResponse();
    void resetResponse();
    int getFileDes() const;
    HttpRequest getRequest();
    sockaddr_storage getAddr() const;
    void setState(Connection::STATE state) { _state = state; }

    HttpRequest _request;
    HttpResponse _response;
};

#endif // CONNECTION_H
