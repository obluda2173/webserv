#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <string.h>
#include <string>
#include <fcntl.h>
#include <unistd.h>

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

class FileBodyProvider : public IBodyProvider {
  private:
    int _fd;
    bool _fdOwned;
    bool _eofReached;

  public:
    explicit FileBodyProvider(const char* filename) : _fd(-1), _fdOwned(true), _eofReached(false) {
        _fd = ::open(filename, O_RDONLY);
        if (_fd < 0) {
            _eofReached = true;
        }
    }

    FileBodyProvider(int existingFd) : _fd(existingFd), _fdOwned(false), _eofReached(false) {}

    virtual ~FileBodyProvider() {
        if (_fdOwned && _fd >= 0) {
            ::close(_fd);
        }
    }

    virtual size_t read(char* buffer, size_t maxSize) {
        if (_fd < 0 || _eofReached) return 0;
        ssize_t n = ::read(_fd, buffer, maxSize);
        if (n > 0) {
            return static_cast<size_t>(n);
        }
        _eofReached = true;
        return 0;
    }

    virtual bool isDone() const { return _eofReached; }
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
