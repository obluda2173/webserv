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
    void reset();
    void parseBuffer();

  public:
    HttpParser();
    ~HttpParser();
    void feed(const char* buffer, size_t length);
    int error(void);
    int ready(void);
    HttpRequest getRequest(void);
};

#endif // HTTPPARSER_H
