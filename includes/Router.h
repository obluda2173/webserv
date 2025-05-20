#ifndef ROUTER_H
#define ROUTER_H

#include "ConfigStructure.h"
#include "HttpRequest.h"
#include "IHandler.h"
#include "IRouter.h"
#include <set>
#include <string>

class Router : public IRouter {
  private:
    std::map< std::string, IHandler* > _hdlrs;
    std::string _defaultSvr;
    std::set< std::string > _svrs;
    std::map< std::string, std::set< std::string > > _svrToLocs;
    std::map< std::string, Route > _routeToRoutes;
    std::string _matchLocations(HttpRequest req);

  public:
    ~Router() {
        for (std::map< std::string, IHandler* >::iterator it = _hdlrs.begin(); it != _hdlrs.end(); it++)
            delete it->second;
    }
    Router(std::map< std::string, IHandler* > hdlrs) : _hdlrs(hdlrs) {}

    Router(IHandler* getHdlr, IHandler* postHdlr, IHandler* delHdlr) {
        _hdlrs["GET"] = getHdlr;
        _hdlrs["POST"] = postHdlr;
        _hdlrs["DELETE"] = delHdlr;
    }

    Router(std::map< std::string, IHandler* > hdlrs, std::string defaultSvr, std::set< std::string > svrs,
           std::map< std::string, std::set< std::string > > svrToLocs, std::map< std::string, Route > routeToRoutes)
        : _hdlrs(hdlrs), _defaultSvr(defaultSvr), _svrs(svrs), _svrToLocs(svrToLocs), _routeToRoutes(routeToRoutes) {};

    void add(std::string svrName, std::string prefix, std::string method, RouteConfig cfg);
    Route match(HttpRequest req);
    void printSvrMap();
};

Router newRouter(std::vector< ServerConfig > svrCfgs, std::map< std::string, IHandler* > hdlrs);
void addSvrToRouter(IRouter* r, ServerConfig svrCfg);
Router newRouterTest();

#endif // ROUTER_H
