#ifndef CONNECTION_H
#define CONNECTION_H

#include "Buffer.h"
#include "HttpResponse.h"
#include "IHttpParser.h"
#include "ISender.h"
#include "ResponseWriter.h"
#include "Route.h"
#include "RouteConfig.h"
#include "SystemSender.h"
#include <fstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

typedef struct UploadContext {
    enum STATE { Validation, Initialising, Uploading, UploadFinished };
    size_t bytesUploaded;
    size_t contentLength;
    std::ofstream* file;
    bool fileExisted;
    UploadContext::STATE state;
    UploadContext()
        : bytesUploaded(0), contentLength(0), file(NULL), fileExisted(false), state(UploadContext::Validation) {}
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
    enum STATE { ReadingHeaders, Routing, Handling, HandleBadRequest, SendResponse, Reset, HandlingCgi };

  private:
    STATE _state;
    sockaddr_storage _addr;
    int _fd;
    int _port;
    IHttpParser* _prsr;
    IResponseWriter* _wrtr;
    ISender* _sender;

  public:
    Route route;
    UploadContext uploadCtx;
    CgiContext cgiCtx;
    Buffer _readBuf;
    Buffer _sendBuf;
    bool _bodyFinished;
    std::string _tempBody;

    ~Connection();
    Connection(sockaddr_storage addr, int fd, int port, IHttpParser* prsr, ISender* = new SystemSender());
    STATE getState() const;
    void setState(Connection::STATE state);
    void readIntoBuf();
    void parseBuf();
    void sendResponse();
    void resetResponse();

    // getter and setter
    int getFileDes() const;
    HttpRequest getRequest();
    sockaddr_storage getAddr() const;
    std::string getReadBuf();
    int getPort() { return _port; }

    HttpRequest _request;
    HttpResponse _response;
};

#endif // CONNECTION_H
