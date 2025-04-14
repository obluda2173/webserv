#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include <sstream>
#include <cstring>
#include <string.h>

#include <IHttpParser.h>

class HttpParser : public IHttpParser {
  private:
    enum State {
      STATE_REQUEST_LINE,
      STATE_HEADERS,
      STATE_BODY,
      STATE_DONE
    };

    State _state;
    std::string _buffer;
    HttpRequest _currentRequest;
    size_t _bodyBytesRead;
    size_t _expectedBodyLength;
    void parseBuffer();

  public:
    HttpParser();                             // hope Kay is fine with this
    ~HttpParser();
    bool feed(char* buffer, size_t length);   // solution for chunk requests
    int error(void);
    int ready(void);
    HttpRequest getRequest(void);
};

#endif // HTTPPARSER_H
