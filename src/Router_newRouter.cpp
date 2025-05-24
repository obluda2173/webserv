#include "Router.h"
#include "ConfigStructure.h"
#include "GetHandler.h"
#include "UploadHandler.h"
#include "utils.h"

void addAllMethods(std::string svrName, std::string prefix, RouteConfig cfg, IRouter* r) {
    r->add(svrName, prefix, "GET", cfg);
    r->add(svrName, prefix, "POST", cfg);
    r->add(svrName, prefix, "DELETE", cfg);
}

void addMethods(std::vector< std::string > methods, std::string svrName, std::string prefix, RouteConfig cfg,
                IRouter* r) {
    for (size_t i = 0; i < methods.size(); i++)
        r->add(svrName, prefix, methods[i], cfg);

    if (methods.empty())
        addAllMethods(svrName, prefix, cfg, r);
}

void addLocations(std::vector< LocationConfig > locations, std::string svrName, IRouter* r) {
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

void addSvrToRouter(IRouter* r, ServerConfig svrCfg) {
    std::vector< std::string > srvNames = svrCfg.serverNames;
    for (std::vector< std::string >::iterator itSvrName = srvNames.begin(); itSvrName != srvNames.end(); itSvrName++) {
        RouteConfig cfg = {svrCfg.common.root,      svrCfg.common.index,
                           svrCfg.common.errorPage, svrCfg.common.clientMaxBody,
                           svrCfg.common.autoindex, std::map< std::string, std::string >()};

        addMethods(svrCfg.common.allowMethods, *itSvrName, "", cfg, r);
        addLocations(svrCfg.locations, *itSvrName, r);
    }
}

std::map< std::string, IRouter* > buildRouters(std::vector< ServerConfig > svrCfgs) {
    std::map< std::string, IRouter* > routers;
    for (size_t i = 0; i < svrCfgs.size(); i++) {
        ServerConfig svrCfg = svrCfgs[i];
        for (std::set< std::pair< std::string, int > >::iterator it = svrCfg.listen.begin(); it != svrCfg.listen.end();
             it++) {
            std::string ip = it->first;
            std::string port = to_string(it->second);
            if (routers.find(ip + ":" + port) != routers.end()) {
                addSvrToRouter(routers[ip + ":" + port], svrCfg);
            } else {
                std::map< std::string, IHandler* > hdlrs;
                hdlrs["GET"] = new GetHandler();
                hdlrs["POST"] = new UploadHandler();
                IRouter* r = new Router(hdlrs);
                addSvrToRouter(r, svrCfg);
                routers[ip + ":" + port] = r;
            }
        }
    }
    return routers;
}

Router newRouter(std::vector< ServerConfig > svrCfgs, std::map< std::string, IHandler* > hdlrs) {
    Router r(hdlrs);
    for (std::vector< ServerConfig >::iterator it = svrCfgs.begin(); it != svrCfgs.end(); ++it) {
        ServerConfig svrCfg = *it;
        addSvrToRouter(&r, svrCfg);
    }
    return r;
}
