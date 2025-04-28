#ifndef ROUTER_H
#define ROUTER_H

#include "HttpRequest.h"
#include <iostream>
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
    std::map<std::string, std::string> svrMap;

  public:
    Router();
    Router(std::map<std::string, std::string> svrCfg) : svrMap(svrCfg) {}
    GetHandler match(HttpRequest req) {
        std::string url = req.headers["host"] + req.uri;
        if (svrMap[url].empty()) {
            url = req.headers["host"] + "/";
        }
        return GetHandler(svrMap[url] + req.uri);
    }
};

#endif // ROUTER_H

// #include "HttpRequest.h"

// class Router {
//   private:
//     int _port;
//     const std::map<std::string, std::vector<std::string>> _svrNameToLocPrefixes;

//   public:
//     Router(std::map<std::string, std::vector<std::string>> svrNameToLocPrefixes);
//     std::string match(const HttpRequest& request);
// };
