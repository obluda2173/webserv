#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <map>
#include <string>
#include <vector>

typedef struct HttpRequest {
  std::string method;                              // GET, POST, DELETE 
  std::string uri;                                 // /path/to/resource
  std::string version;                             // HTTP/1.1
  std::map<std::string, std::string> headers;      // key-value pairs for headers
  char* body;                                		// request body (if any)
  std::map<std::string, std::string> queryParams;  // query parameters from URI
} HttpRequest;

#endif // HTTPREQUEST_H
