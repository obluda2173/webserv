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
    std::string _defaultSvr;
    std::set<std::string> _svrs;
    std::map<std::string, std::string> _uriToDirPath;
    std::map<std::string, std::set<std::string>> _uriToAllowedMethod;
    std::map<std::string, std::set<std::string>> _svrToLocs;

  public:
    Router() {}
    Router(std::string defaultSvr, std::map<std::string, std::string> routes,
           std::map<std::string, std::set<std::string>> routeAllowedMethods, std::set<std::string> svrs,
           std::map<std::string, std::set<std::string>> svrToLocs)
        : _defaultSvr(defaultSvr), _svrs(svrs), _uriToDirPath(routes), _uriToAllowedMethod(routeAllowedMethods),
          _svrToLocs(svrToLocs) {};

    void add(std::string svrName, std::string prefix, std::string root, std::vector<std::string> allowedMethods);
    ExecutionInfo match(HttpRequest req);
    void printSvrMap();
};

Router newRouter(std::vector<ServerConfig>);
Router newRouterTest();

#endif // ROUTER_H
