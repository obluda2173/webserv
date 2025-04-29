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
    std::vector<std::string> locs = {"/css/", "/images/", "/css/styles/"};

  public:
    Router();
    Router(std::map<std::string, std::string> svrCfg) : _svrMap(svrCfg) {}

    GetHandler match(HttpRequest req) {
        std::string url = req.headers["host"] + req.uri;
        if (!_svrMap[url].empty())
            return GetHandler(_svrMap[url] + req.uri);

        for (size_t i = 0; i < locs.size(); i++) {
            if (req.uri.compare(0, locs[i].length(), locs[i]) == 0) {
                url = req.headers["host"] + locs[i];
            }
        }
        if (!_svrMap[url].empty())
            return GetHandler(_svrMap[url] + req.uri);

        url = req.headers["host"] + "/";
        if (!_svrMap[url].empty())
            return GetHandler(_svrMap[url] + req.uri);

        req.headers["host"] = _svrMap["default"];
        return match(req);
    }
};

#endif // ROUTER_H
