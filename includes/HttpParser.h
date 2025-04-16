#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include <sstream>
#include <cstring>
#include <string.h>
#include <iostream>

#include <IHttpParser.h>

#define DO_NOTHING 0;
#define DO_RETURN 1;
#define DO_BREAK 2;

class HttpParser : public IHttpParser {
  private:
    enum State {
      STATE_REQUEST_LINE,
      STATE_HEADERS,
      STATE_BODY,
      STATE_CHUNKED_BODY,
      STATE_DONE,
      STATE_ERROR
    };

    State _state;
    std::string _buffer;
    HttpRequest _currentRequest;
    size_t _expectedBodyLength;
    bool _isChunked;
    size_t _chunkSize;
    int _chunkState;
    void reset();
    void parseBuffer();
    int handleRequestHeaders(); // 0 for noting. 1 for return, 2 for break
    int handleBodyContent();
    int handleBodyChunk();

  public:
    HttpParser();                             // hope Kay is fine with this
    ~HttpParser();
    bool feed(const char* buffer, size_t length);   // solution for chunk requests
    int error(void);
    int ready(void);
    HttpRequest getRequest(void);
};

#endif // HTTPPARSER_H
