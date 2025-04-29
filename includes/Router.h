#ifndef ROUTER_H
#define ROUTER_H

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
    Router();
    Router(std::map<std::string, std::string> svrCfg) : _svrMap(svrCfg) {
        _allLocs["example.com"] = {"/images/", "/css/scripts/", "/css/", "/css/styles/"};
        _allLocs["test.com"] = {"/css/", "/js/", "/images/"};
    }
    void add(std::string url, std::string root) { _svrMap[url] = root; }

    GetHandler match(HttpRequest req);
};

Router newRouter();

#endif // ROUTER_H
