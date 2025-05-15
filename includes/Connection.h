#ifndef CONNECTION_H
#define CONNECTION_H

#include "Buffer.h"
#include "HttpResponse.h"
#include "IHttpParser.h"
#include "ISender.h"
#include "ResponseWriter.h"
#include "RouteConfig.h"
#include "SystemSender.h"
#include <fstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

typedef struct UploadContext {
    size_t bytesUploaded;
    size_t contentLength;
    std::ofstream* file;
    bool fileExisted;
    std::string state;
    UploadContext() : bytesUploaded(0), contentLength(0), file(NULL), fileExisted(false), state("") {}
} UploadContext;

struct CgiContext {
    pid_t cgiPid;               // Store CGI process ID
    int cgiPipeFd;              // Store CGI output pipe
    std::string cgiOutput;      // Accumulate CGI output
    RouteConfig cgiRouteConfig; // Store RouteConfig for response
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

    // std::vector< char > _sendBuf;
    // size_t _sendBufUsedSize;

  public:
    UploadContext uploadCtx;
    CgiContext cgiCtx;
    Buffer _readBuf;
    Buffer _sendBuf;

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
