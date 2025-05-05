#ifndef RESPONSEWRITER_H
#define RESPONSEWRITER_H

#include "HttpResponse.h"
#include <string>

class IResponseWriter {
  public:
    virtual ~IResponseWriter() {}
    virtual size_t write(char* buffer, size_t maxSize) = 0;
};

class ResponseWriter : public IResponseWriter {
  private:
    HttpResponse _resp;
    std::string _headerString;
    bool _headersWritten;
    bool _headersCopied;
    void _writeHeaders();
    void _writeStatusLine();
    void _writeBody(char* buffer, size_t maxSize);

  public:
    static const std::string CRLF;
    static const std::string WS;

    ResponseWriter(HttpResponse& resp) : _resp(resp), _headersWritten(false), _headersCopied(false) {}
    virtual size_t write(char* buffer, size_t maxSize);
};

#endif // RESPONSEWRITER_H
