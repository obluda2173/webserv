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
  public:
    enum STATE { FormatHeaders, WritingHeaders, WritingBody };

  private:
    STATE _state;
    HttpResponse _resp;
    std::string _headers;
    bool _headersWritten;
    void _formatHeaders();
    void _writeStatusLine();
    size_t _writeHeaders(char* buffer, size_t maxSize);

  public:
    static const std::string CRLF;
    static const std::string WS;

    ResponseWriter(HttpResponse& resp) : _state(FormatHeaders), _resp(resp), _headersWritten(false) {}

    virtual size_t write(char* buffer, size_t maxSize);
};

#endif // RESPONSEWRITER_H
