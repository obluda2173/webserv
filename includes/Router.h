#ifndef ROUTER_H
#define ROUTER_H

#include "ConfigStructure.h"
#include "HttpRequest.h"
#include <set>
#include <string>
#include <unordered_map>

struct RouteConfig {
    std::string root;
    // std::vector<std::string> index;
    // std::map<int, std::string> errorPage;
    // bool autoindex;
};

class Response {};

class IHandler {
  public:
    virtual ~IHandler() {}
    virtual Response handle(const HttpRequest& req, const RouteConfig& config) = 0;
};

class Handler : public IHandler {
  public:
    Handler(std::string type) : type(type) {}
    ~Handler() {}
    std::string type;
    virtual Response handle(const HttpRequest&, const RouteConfig&) { return Response{}; };
};

struct Route {
    std::unordered_map<std::string, IHandler&> hdlrs;
    RouteConfig cfg;
};

class Router {
  private:
    std::map<std::string, IHandler*> _hdlrs;
    std::string _defaultSvr;
    std::set<std::string> _svrs;
    std::map<std::string, std::string> _routeToRoot;
    std::map<std::string, std::set<std::string>> _svrToLocs;
    std::map<std::string, Route> _routeToRoutes;
    std::string _matchLocations(HttpRequest req);

  public:
    ~Router() {
        for (std::map<std::string, IHandler*>::iterator it = _hdlrs.begin(); it != _hdlrs.end(); it++)
            delete it->second;
    }
    Router() {
        _hdlrs["GET"] = new Handler("GET");
        _hdlrs["POST"] = new Handler("POST");
        _hdlrs["DELETE"] = new Handler("DELETE");
    }
    Router(std::string defaultSvr, std::map<std::string, std::string> routes, std::set<std::string> svrs,
           std::map<std::string, std::set<std::string>> svrToLocs, std::map<std::string, Route> routeToRoutes)
        : _defaultSvr(defaultSvr), _svrs(svrs), _routeToRoot(routes), _svrToLocs(svrToLocs), _routeToRoutes(routeToRoutes) {};

    void add(std::string svrName, std::string prefix, std::string root, std::vector<std::string> allowedMethods);
    Route match(HttpRequest req);
    void printSvrMap();
};

Router newRouter(std::vector<ServerConfig>);
Router newRouterTest();

#endif // ROUTER_H
