#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <map>
#include <string>

typedef struct HttpRequest {
    std::string method;                         // GET, POST, DELETE
    std::string uri;                            // /path/to/resource
    std::string query;
    std::string version;                        // HTTP/1.1
    std::map<std::string, std::string> headers; // key-value pairs for headers
} HttpRequest;

#endif // HTTPREQUEST_H
