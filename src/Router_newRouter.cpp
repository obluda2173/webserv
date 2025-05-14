#include "Router.h"
#include "ConfigStructure.h"

void addAllMethods(std::string svrName, std::string prefix, RouteConfig cfg, Router& r) {
    r.add(svrName, prefix, "GET", cfg);
    r.add(svrName, prefix, "POST", cfg);
    r.add(svrName, prefix, "DELETE", cfg);
}

void addMethods(std::vector< std::string > methods, std::string svrName, std::string prefix, RouteConfig cfg,
                Router& r) {
    for (size_t i = 0; i < methods.size(); i++)
        r.add(svrName, prefix, methods[i], cfg);

    if (methods.empty())
        addAllMethods(svrName, prefix, cfg, r);
}

void addLocations(std::vector< LocationConfig > locations, std::string svrName, Router& r) {
    std::set< std::string > presentLocs;
    for (std::vector< LocationConfig >::iterator itLoc = locations.begin(); itLoc != locations.end(); ++itLoc) {
        if (presentLocs.find(itLoc->prefix) != presentLocs.end())
            continue;
        presentLocs.insert(itLoc->prefix);
        RouteConfig cfg = {itLoc->common.root,          itLoc->common.index,     itLoc->common.errorPage,
                           itLoc->common.clientMaxBody, itLoc->common.autoindex, itLoc->cgi};
        addMethods(itLoc->common.allowMethods, svrName, itLoc->prefix, cfg, r);
    }
}

void addSvrToRouter(Router& r, ServerConfig svrCfg) {
    std::vector< std::string > srvNames = svrCfg.serverNames;
    for (std::vector< std::string >::iterator itSvrName = srvNames.begin(); itSvrName != srvNames.end(); itSvrName++) {
        RouteConfig cfg = {svrCfg.common.root,      svrCfg.common.index,
                           svrCfg.common.errorPage, svrCfg.common.clientMaxBody,
                           svrCfg.common.autoindex, std::map< std::string, std::string >()};

        addMethods(svrCfg.common.allowMethods, *itSvrName, "", cfg, r);
        addLocations(svrCfg.locations, *itSvrName, r);
    }
}

Router newRouter(std::vector< ServerConfig > svrCfgs, std::map< std::string, IHandler* > hdlrs) {
    Router r(hdlrs);
    for (std::vector< ServerConfig >::iterator it = svrCfgs.begin(); it != svrCfgs.end(); ++it) {
        ServerConfig svrCfg = *it;
        addSvrToRouter(r, svrCfg);
    }
    return r;
}
