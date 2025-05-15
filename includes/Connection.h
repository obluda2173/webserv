#ifndef CONNECTION_H
#define CONNECTION_H

#include "HttpResponse.h"
#include "IHttpParser.h"
#include "ResponseWriter.h"
#include "RouteConfig.h"
#include <fstream>
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

typedef struct UploadContext {
    size_t bytesUploaded;
    size_t contentLength;
    std::ofstream* file;
    bool fileExisted;
    UploadContext() : bytesUploaded(0), contentLength(0), file(NULL), fileExisted(false) {}
} UploadContext;

struct CgiContext {
    pid_t cgiPid;               // Store CGI process ID
    int cgiPipeFd;              // Store CGI output pipe
    std::string cgiOutput;      // Accumulate CGI output
    RouteConfig cgiRouteConfig; // Store RouteConfig for response
    CgiContext() : cgiPid(-1), cgiPipeFd(-1) {}
};

class Buffer {
  private:
    std::vector< char > _content;
    size_t _size;

  public:
    Buffer() {
        _content.resize(1024);
        _size = 0;
    }

    void recvNew(int fd) {
        ssize_t r = recv(fd, _content.data() + _size, _content.size() - _size, 0);
        _size += r;
        std::cout << _size << std::endl;
    }

    void advance(size_t count) {
        memmove(_content.data(), _content.data() + count, _size - count);
        _size -= count;
    }

    void clear() { _size = 0; }
    size_t size() const { return _size; }
    void assign(std::string s) {
        if (s.length() > _content.size()) {
            std::cout << "string is bigger than readBuf" << std::endl;
            _exit(1);
        }
        _content.assign(s.begin(), s.end());
        _size = s.length();
    }
    char* data() { return _content.data(); }
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
    std::vector< char > _sendBuf;
    size_t _sendBufUsedSize;

  public:
    UploadContext uploadCtx;
    CgiContext cgiCtx;
    Buffer _readBuf;

    ~Connection();
    Connection(sockaddr_storage addr, int fd, IHttpParser* prsr, ISender* = new SystemSender());
    STATE getState() const;
    void readIntoBuf();
    void parseBuf();
    void sendResponse();
    void resetResponse();

    // getter and setter
    int getFileDes() const;
    HttpRequest getRequest();
    sockaddr_storage getAddr() const;
    void setState(Connection::STATE state);
    void setReadBuf(std::string s);
    std::string getReadBuf();

    HttpRequest _request;
    HttpResponse _response;
};

#endif // CONNECTION_H
