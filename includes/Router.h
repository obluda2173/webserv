#ifndef ROUTER_H
#define ROUTER_H

#include "HttpRequest.h"

class Router {
  private:
    int _port;
    const std::map<std::string, std::vector<std::string>> _svrNameToLocPrefixes;

  public:
    Router(std::map<std::string, std::vector<std::string>> svrNameToLocPrefixes);
    std::pair<std::string, std::string> match(const HttpRequest& request);
};

#endif // ROUTER_H
