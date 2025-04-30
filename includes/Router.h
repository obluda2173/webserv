#ifndef ROUTER_H
#define ROUTER_H

#include "ConfigStructure.h"
#include "HttpRequest.h"
#include <set>
#include <string>

struct ExecutionInfo {
    std::string path;
    std::string execType;
};

class Router {
  private:
    std::map<std::string, std::string> _routes;
    std::map<std::string, std::set<std::string>> _routeAllowedMethods;
    std::map<std::string, std::vector<std::string>> _svrToLocs;

  public:
    Router() {}
    Router(std::map<std::string, std::string> routes, std::map<std::string, std::set<std::string>> routeAllowedMethods,
           std::map<std::string, std::vector<std::string>> svrToLocs)
        : _routes(routes), _routeAllowedMethods(routeAllowedMethods), _svrToLocs(svrToLocs) {};

    void add(std::string svrName, std::string prefix, std::string root, std::vector<std::string> allowedMethods);
    ExecutionInfo match(HttpRequest req);
    void printSvrMap();
};

Router newRouter(std::vector<ServerConfig>);
Router newRouterTest();

#endif // ROUTER_H
