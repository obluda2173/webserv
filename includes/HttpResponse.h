#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <fcntl.h>
#include <fstream>
#include <map>
#include <iostream>
#include <string.h>
#include <string>
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
    std::ifstream* _file;
    bool _eofReached;

  public:
    explicit FileBodyProvider(const char* filename) : _file(new std::ifstream(filename, std::ios_base::in)) {
        if (!_file || !_file->is_open()) {
            std::cout << "Error opening file" << std::endl;
        }
        _eofReached = false;
    }

    // FileBodyProvider(int existingFd) : _file(NULL), _eofReached(false) {}

    virtual ~FileBodyProvider() {
        _file->close();
        delete _file;
    }

    virtual size_t read(char* buffer, size_t maxSize) {
        if (_eofReached)
            return 0;
        _file->read(buffer, maxSize);
        std::streamsize n = _file->gcount();
        _eofReached = (n == 0 || _file->eof() || _file->fail());
        return static_cast< size_t >(n);
    }

    virtual bool isDone() const { return _eofReached; }
};

typedef struct HttpResponse {
    int statusCode;
    std::string version;
    IBodyProvider* body;
    std::string statusMessage;
    std::string contentType;
    int contentLength;
    std::string contentLanguage;
    std::map<std::string, std::string> headers;

    HttpResponse() : body(NULL), contentLength(0) {}
} HttpResponse;

#endif
