#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <map>
#include <string>
#include <vector>

typedef struct HttpRequest {
  std::string method;                                            // GET, POST, DELETE 
  std::string uri;                                               // /path/to/resource
  std::string version;                                           // HTTP/1.1
  std::vector<std::pair<std::string, std::string> > headers;     // key-value pairs for headers
  bool hasBody;                                                  // Indicates if body is expected
} HttpRequest;

#endif // HTTPREQUEST_H
