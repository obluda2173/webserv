#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <string>

typedef struct HttpResponse {
    int statusCode;
    std::string version;
    std::string body;
    std::string statusMessage;
    bool isClosed;
    std::string contentType;
    int contentLength;
    std::string contentLanguage;
    bool isRange;
    bool isChunked;

    HttpResponse()
        : statusCode(0), version(""), body(""), statusMessage(""), isClosed(false), contentType(""), contentLength(0),
          contentLanguage(""), isRange(false), isChunked(false) {}
} HttpResponse;

#endif
