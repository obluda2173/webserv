#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include <sstream>
#include <cstring>
#include <string.h>
#include <iostream>

#include <IHttpParser.h>

class HttpParser : public IHttpParser {
  private:
    enum State {
      STATE_REQUEST_LINE,
      STATE_HEADERS,
      STATE_DONE,
      STATE_ERROR
    };

    State _state;
    std::string _buffer;
    HttpRequest _currentRequest;
    size_t _totalProcessedSize;
    size_t _maxHeaderKeySize;   // 256B
    size_t _maxHeaderSize;      // 4KB
    bool _extractNextLine(std::string& line);
    void _parseRequestLine(const std::string& line);
    void _parseHeader(const std::string& line);
    void _handlePartialLine();
    void _reset();
    void _parseBuffer();
    bool _requestLineValidation(const std::string& method, const std::string& uri, const std::string& version);
    bool _headerLineValidation(const std::string& key, const std::string& value);

  public:
    HttpParser();
    HttpParser(size_t maxHeaderKeySize, size_t maxHeaderSize);
    ~HttpParser();
    void feed(const char* buffer, size_t length);
    int error(void);
    int ready(void);
    HttpRequest getRequest(void);
};

#endif // HTTPPARSER_H
