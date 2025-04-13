#ifndef IHTTPPARSER_H
#define IHTTPPARSER_H

#include <HttpRequest.h>

class IHttpParser {
  public:
    virtual ~IHttpParser() = default;
    virtual HttpRequest parse(const std::string& request) = 0;
};

#endif // IHTTPPARSER_H
