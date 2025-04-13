#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <map>
#include <string>
#include <vector>

struct HttpRequest {
  std::string method;                              // GET, POST
  std::string uri;                                 // /path/to/resource
  std::string version;                             // HTTP/1.1
  std::map<std::string, std::string> headers;      // key-value pairs for headers
  std::string body;                                // request body (if any)
  std::map<std::string, std::string> queryParams;  // query parameters from URI
};

#endif // HTTPREQUEST_H
