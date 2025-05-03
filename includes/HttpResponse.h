#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <string>

typedef struct HttpResponse {
    int statusCode;
    std::string response;
    std::string statusMessage;
    bool isClosed = false;
    std::string contentType;
    std::string contentLength;
    std::string contentLanguage;
    bool isRange = false;
    bool isChunked = false;
} HttpResponseMessage;

#endif
