#ifndef ROUTER_H
#define ROUTER_H

#include "ConfigStructure.h"
#include "HttpRequest.h"
#include <set>
#include <string>

class ExecutionInfo {
  private:
    std::string _dirPath;
    std::string _execType;

  public:
    ExecutionInfo(std::string dirPath, std::string execType) : _dirPath(dirPath), _execType(execType) {}
    std::string getDirPath() { return _dirPath; }
    std::string getExecType() { return _execType; }
};

// class ExecutionInfo : public IExecutionInfo {};

// class ErrorExecutionInfo : public IExecutionInfo {};

class Router {
  private:
    std::string _defaultSvr;
    std::set<std::string> _svrs;
    std::map<std::string, std::string> _routeToDirPath;
    std::map<std::string, std::set<std::string>> _routeToAllowedMethod;
    std::map<std::string, std::set<std::string>> _svrToLocs;
    ExecutionInfo _checkAllowedMethods(std::string route, HttpRequest req);
    std::string _matchLocations(HttpRequest req);

  public:
    Router() {}
    Router(std::string defaultSvr, std::map<std::string, std::string> routes,
           std::map<std::string, std::set<std::string>> routeAllowedMethods, std::set<std::string> svrs,
           std::map<std::string, std::set<std::string>> svrToLocs)
        : _defaultSvr(defaultSvr), _svrs(svrs), _routeToDirPath(routes), _routeToAllowedMethod(routeAllowedMethods),
          _svrToLocs(svrToLocs) {};

    void add(std::string svrName, std::string prefix, std::string root, std::vector<std::string> allowedMethods);
    ExecutionInfo match(HttpRequest req);
    void printSvrMap();
};

Router newRouter(std::vector<ServerConfig>);
Router newRouterTest();

#endif // ROUTER_H
