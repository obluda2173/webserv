#ifndef ROUTER_H
#define ROUTER_H

#include "HttpRequest.h"
#include <iostream>
#include <set>
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
    std::vector<std::string> locs = {"/css/", "/images/"};

  public:
    Router();
    Router(std::map<std::string, std::string> svrCfg) : _svrMap(svrCfg) {}

    GetHandler match(HttpRequest req) {
        std::string url = req.headers["host"] + req.uri;

        if (req.uri == "/css/themes/") {
            url = req.headers["host"] + "/css/";
        }

        if (req.uri == "/images/themes/") {
            url = req.headers["host"] + "/images/";
        }

        if (req.uri == "/images/screenshots/") {
            url = req.headers["host"] + "/images/";
        }

        if (_svrMap[url].empty()) {
            url = req.headers["host"] + "/";
            if (_svrMap[url].empty()) {
                req.headers["host"] = _svrMap["default"];
                return match(req);
            }
        }
        return GetHandler(_svrMap[url] + req.uri);
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
