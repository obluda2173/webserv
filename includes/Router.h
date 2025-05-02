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
    virtual ~IHandler() {}
    virtual Response handle(const HttpRequest& req, const RouteConfig& config) = 0;
};

struct Route {
    std::unordered_map<std::string, IHandler&> handler;
    RouteConfig& config;
};

class ExecutionInfo {
  private:
    std::string _root;
    std::string _execType;

  public:
    ExecutionInfo(std::string root, std::string execType) : _root(root), _execType(execType) {}
    std::string getRoot() { return _root; }
    std::string getExecType() { return _execType; }
};

// class ExecutionInfo : public IExecutionInfo {};

// class ErrorExecutionInfo : public IExecutionInfo {};

class Router {
  private:
    std::string _defaultSvr;
    std::set<std::string> _svrs;
    std::map<std::string, std::string> _routeToRoot;
    std::map<std::string, std::set<std::string>> _routeToAllowedMethod;
    std::map<std::string, std::set<std::string>> _svrToLocs;
    ExecutionInfo _checkAllowedMethods(std::string route, HttpRequest req);
    std::string _matchLocations(HttpRequest req);

  public:
    Router() {}
    Router(std::string defaultSvr, std::map<std::string, std::string> routes,
           std::map<std::string, std::set<std::string>> routeAllowedMethods, std::set<std::string> svrs,
           std::map<std::string, std::set<std::string>> svrToLocs)
        : _defaultSvr(defaultSvr), _svrs(svrs), _routeToRoot(routes), _routeToAllowedMethod(routeAllowedMethods),
          _svrToLocs(svrToLocs) {};

    void add(std::string svrName, std::string prefix, std::string root, std::vector<std::string> allowedMethods);
    ExecutionInfo match(HttpRequest req);
    void printSvrMap();
};

Router newRouter(std::vector<ServerConfig>);
Router newRouterTest();

#endif // ROUTER_H
