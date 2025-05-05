#ifndef RESPONSEWRITER_H
#define RESPONSEWRITER_H

#include "HttpResponse.h"
#include <string>

class IResponseWriter {
  public:
    virtual ~IResponseWriter() {}
    virtual int write(char* buffer, int maxSize) = 0;
};

class ResponseWriter : public IResponseWriter {
  private:
    HttpResponse _resp;
    std::string _respString;
    void _writeHeaders();
    void _writeStatusLine();
    void _writeBody();

  public:
    static const std::string CLRF;
    static const std::string WS;

    ResponseWriter(HttpResponse& resp) : _resp(resp) {}
    virtual int write(char* buffer, int maxSize);
};

#endif // RESPONSEWRITER_H
