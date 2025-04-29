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
    std::map<std::string, std::string> _svrMap;
    std::map<std::string, std::vector<std::string>> _allLocs;

  public:
    void add(std::string svrName, std::string prefix, std::string root) {
        _svrMap[svrName + prefix] = root;
        _allLocs[svrName].push_back(prefix);
    }

    ExecutionInfo match(HttpRequest req);
    void printSvrMap();
};

Router newRouter(std::vector<ServerConfig>);

#endif // ROUTER_H
