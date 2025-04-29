#ifndef ROUTER_H
#define ROUTER_H

#include "ConfigStructure.h"
#include "HttpRequest.h"
#include <string>

class GetHandler {
  private:
    std::string _path;
    std::string _defaultPath;

  public:
    GetHandler(std::string path) : _path(path) {}
    std::string getPath() { return _path; }
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

    std::string match(HttpRequest req);
    void printSvrMap();
};

Router newRouter(std::vector<ServerConfig>);

#endif // ROUTER_H
