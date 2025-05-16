#ifndef IHTTPPARSER_H
#define IHTTPPARSER_H

#include <HttpRequest.h>

// 1. In our implementation of the server, we demand the HttpParser to reset if the requests is got
// 2. feed is always called with length 1 in our implementation. Might prompt for an interface change
class IHttpParser {
  public:
    virtual ~IHttpParser() {}
    virtual void feed(const char* buffer, size_t length) = 0; // !!! buffer needs to be NULL-terminated !!!
    virtual int error(void) = 0;
    virtual int ready(void) = 0;
    virtual HttpRequest getRequest(void) = 0;
};

#endif // IHTTPPARSER_H
