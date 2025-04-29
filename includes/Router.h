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
    void add(std::string svrName, std::string prefix, std::string root, std::vector<std::string> allowedMethods) {
        _routes[svrName + prefix] = root;
        _routeAllowedMethods[svrName + prefix] = std::set(allowedMethods.begin(), allowedMethods.end());
        if (allowedMethods.empty()) {
            _routeAllowedMethods[svrName + prefix].insert("GET");
            _routeAllowedMethods[svrName + prefix].insert("POST");
            _routeAllowedMethods[svrName + prefix].insert("DELETE");
        }
        _svrToLocs[svrName].push_back(prefix);
    }

    ExecutionInfo match(HttpRequest req);
    void printSvrMap();
};

Router newRouter(std::vector<ServerConfig>);

#endif // ROUTER_H
