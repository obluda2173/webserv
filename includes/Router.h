#ifndef ROUTER_H
#define ROUTER_H

#include "ConfigStructure.h"
#include "HttpRequest.h"
#include <string>

struct ExecutionInfo {
    std::string path;
    std::string execType;
};

class Router {
  private:
    std::map<std::string, std::string> _routes;
    std::map<std::string, std::vector<std::string>> _svrToLocs;

  public:
    void add(std::string svrName, std::string prefix, std::string root) {
        _routes[svrName + prefix] = root;
        _svrToLocs[svrName].push_back(prefix);
    }

    ExecutionInfo match(HttpRequest req);
    void printSvrMap();
};

Router newRouter(std::vector<ServerConfig>);

#endif // ROUTER_H
