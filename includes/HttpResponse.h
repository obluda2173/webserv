#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <string>

typedef struct HttpResponse {
    std::string response;
    int statusCode;
} HttpResponse;

#endif // HTTPRESPONSE_H
