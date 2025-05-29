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

typedef struct BodyContext {
    enum TYPE { Undetermined, ContentLength, TransferEncoding };
    BodyContext::TYPE type;

    // Transfer-Encoding related
    enum TE_STAGE { ReadingChunkSize, ReadingChunk, VerifyCarriageReturn };
    BodyContext::TE_STAGE _transferEncodingState;
    size_t _chunkRestSize;
    size_t _currentChunkSize;
    std::string _bodyBuf;

    // ContentLength related
    size_t bytesReceived;
    size_t contentLength;
    BodyContext()
        : type(Undetermined), _transferEncodingState(ReadingChunkSize), _chunkRestSize(0), _currentChunkSize(0),
          _bodyBuf(""), bytesReceived(0), contentLength(0) {}
} BodyContext;

typedef struct UploadContext {
    enum STATE { Validation, Initialising, Uploading, UploadFinished };
    std::ofstream* file;
    bool fileExisted;
    UploadContext::STATE state;
    UploadContext() : file(NULL), fileExisted(false), state(UploadContext::Validation) {}
} UploadContext;

struct CgiContext {
    enum STATE { Forking, WritingStdin, ReadingStdout, Exited };
    CgiContext::STATE state;
    int cgiPipeStdin;
    int cgiPipeStdout;
    pid_t cgiPid;               // Store CGI process ID
    std::string cgiOutput;      // Accumulate CGI output
    RouteConfig cgiRouteConfig; // Store RouteConfig for response
    CgiContext() : state(CgiContext::Forking), cgiPipeStdin(-1), cgiPipeStdout(-1), cgiPid(-1) {}
};

class Connection {
  public:
    enum STATE { ReadingHeaders, Routing, Redirecting, Handling, HandleBadRequest, SendResponse, Reset, HandlingCgi };

  private:
    STATE _state;
    sockaddr_storage _addr; // address of client
    int _fd;
    std::string _addrPort; // port of server that was hit with the request
    IHttpParser* _prsr;
    IResponseWriter* _wrtr;
    ISender* _sender;

  public:
    Route route;
    UploadContext uploadCtx;
    CgiContext cgiCtx;
    Buffer _readBuf;
    Buffer _sendBuf;
    BodyContext bodyCtx;
    std::map< std::string, size_t > _sessionIdsDataBase;
    std::map< std::string, std::string > cookies;
    bool _bodyFinished;
    std::string _tempBody;
    IHandler* _hdlr;

    ~Connection();

    Connection(sockaddr_storage addr, int fd, std::string addrPort, IHttpParser* prsr,
               ISender* sender = new SystemSender());
    STATE getState() const;
    void setState(Connection::STATE state);
    ssize_t readIntoBuf();
    void parseBuf();
    ssize_t sendResponse();
    void resetResponse();
    void checkRoute();
    void resetCGI();

    // getter and setter
    int getFileDes() const;
    HttpRequest getRequest();
    sockaddr_storage getAddr() const;
    std::string getReadBuf();
    std::string getAddrPort() { return _addrPort; }

    HttpRequest _request;
    HttpResponse _response;
};

#endif // CONNECTION_H
