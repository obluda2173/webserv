#ifndef ROUTER_H
#define ROUTER_H

#include "HttpRequest.h"
#include "ConfigStructure.h"

class Router {
  private:
    const std::vector<ServerConfig>& _servers;
    std::map<int, std::vector<const ServerConfig*>> _byPort;
  
  public:
    Router(const std::vector<ServerConfig>& servers);
    std::pair<const ServerConfig*, const LocationConfig*> match(const HttpRequest& request, int );
};

#endif // ROUTER_H
