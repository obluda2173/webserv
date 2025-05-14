#ifndef CONNECTION_H
#define CONNECTION_H

#include "HttpResponse.h"
#include "IHttpParser.h"
#include "ResponseWriter.h"
#include <fstream>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include "RouteConfig.h"

#define READ_SIZE 2048

class ISender {
  public:
    virtual ~ISender() {}
    virtual size_t _send(int fd, char* buf, size_t size) = 0;
};

class SystemSender : public ISender {
    virtual size_t _send(int fd, char* buf, size_t n) { return send(fd, buf, n, 0); }
};

typedef struct UploadContext {
    size_t bytesUploaded;
    size_t contentLength;
    std::ofstream* file;
    bool fileExisted;
    UploadContext() : bytesUploaded(0), contentLength(0), file(NULL), fileExisted(false) {}
} UploadContext;

struct CgiContext {
    pid_t cgiPid;                       // Store CGI process ID
    int cgiPipeFd;                      // Store CGI output pipe
    std::string cgiOutput;              // Accumulate CGI output
    RouteConfig cgiRouteConfig;         // Store RouteConfig for response
    CgiContext() : cgiPid(-1), cgiPipeFd(-1) {}
};

class Connection {
  public:
    enum STATE { ReadingHeaders, Handling, HandleBadRequest, SendResponse, Reset, HandlingCgi };

  private:
    STATE _state;
    sockaddr_storage _addr;
    int _fd;
    IHttpParser* _prsr;
    IResponseWriter* _wrtr;
    ISender* _sender;
    std::vector< char > _readBuf;
    std::vector< char > _sendBuf;
    size_t _sendBufUsedSize;

  public:
    UploadContext uploadCtx;
    CgiContext cgiCtx;
    size_t _readBufUsedSize;
    ~Connection();
    Connection(sockaddr_storage addr, int fd, IHttpParser* prsr, ISender* = new SystemSender());
    STATE getState() const;
    void readIntoBuf();
    void parseBuf();
    void sendResponse();
    void resetResponse();
    int getFileDes() const;
    HttpRequest getRequest();
    sockaddr_storage getAddr() const;
    void setState(Connection::STATE state) { _state = state; }
    void setReadBuf(std::string s) {
        if (s.length() > _readBuf.size()) {
            std::cout << "string is bigger than readBuf" << std::endl;
            _exit(1);
        }
        _readBuf.assign(s.begin(), s.end());
        _readBufUsedSize = s.length();
    }
    std::string getReadBuf() { return std::string(_readBuf.data(), _readBufUsedSize); }

    HttpRequest _request;
    HttpResponse _response;
};

#endif // CONNECTION_H
