#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <string.h>
#include <string>

class IBodyProvider {
  public:
    virtual ~IBodyProvider() {}
    virtual size_t read(char* buffer, size_t maxSize) = 0;
    virtual bool isDone() const = 0;
};

class StringBodyProvider : public IBodyProvider {
  private:
    std::string data;
    size_t position;

  public:
    StringBodyProvider(const std::string& content) : data(content), position(0) {}
    virtual size_t read(char* buffer, size_t maxSize) {
        if (position >= data.length()) {
            return 0;
        }
        size_t bytesToCopy = std::min(maxSize, data.length() - position);
        memcpy(buffer, data.c_str() + position, bytesToCopy);
        position += bytesToCopy;
        return bytesToCopy;
    }

    virtual bool isDone() const { return position >= data.length(); }
};

typedef struct HttpResponse {
    int statusCode;
    std::string version;
    IBodyProvider* body;
    std::string statusMessage;
    bool isClosed;
    std::string contentType;
    int contentLength;
    std::string contentLanguage;
    bool isRange;
    bool isChunked;

    HttpResponse()
        : statusCode(0), version(""), body(NULL), statusMessage(""), isClosed(false), contentType(""), contentLength(0),
          contentLanguage(""), isRange(false), isChunked(false) {}
} HttpResponse;

#endif
