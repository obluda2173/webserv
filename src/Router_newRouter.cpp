#include "Router.h"
#include "CgiHandler.h"
#include "ConfigStructure.h"
#include "GetHandler.h"
#include "UploadHandler.h"
#include "DeleteHandler.h"
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

    if (!cfg.cgi.empty())
        r->add(svrName, prefix, "CGI", cfg);

    if (methods.empty())
        addAllMethods(svrName, prefix, cfg, r);
}

void addLocations(ServerConfig svrCfg, std::string svrName, IRouter* r) {
    std::vector< LocationConfig > locations = svrCfg.locations;
    std::set< std::string > presentLocs;
    for (std::vector< LocationConfig >::iterator itLoc = locations.begin(); itLoc != locations.end(); ++itLoc) {
        if (presentLocs.find(itLoc->prefix) != presentLocs.end())
            continue;
        presentLocs.insert(itLoc->prefix);
        RouteConfig cfg;
        cfg.root = itLoc->common.root.empty() ? svrCfg.common.root : itLoc->common.root;
        cfg.index = itLoc->common.index.empty() ? svrCfg.common.index : itLoc->common.index;
        cfg.errorPage = itLoc->common.errorPage.empty() ? svrCfg.common.errorPage : itLoc->common.errorPage;
        cfg.clientMaxBody = itLoc->common.clientMaxBody;
        cfg.autoindex = (itLoc->common.autoindex == "on") ? true : false;
        cfg.cgi = itLoc->cgi;
        cfg.redirect = itLoc->redirect;

        addMethods(itLoc->common.allowMethods, svrName, itLoc->prefix, cfg, r);
    }
}

void addSvrToRouter(IRouter* r, ServerConfig svrCfg, std::string& port) {
    std::vector< std::string > srvNames = svrCfg.serverNames;
    for (std::vector< std::string >::iterator itSvrName = srvNames.begin(); itSvrName != srvNames.end(); itSvrName++) {
        RouteConfig cfg;
        cfg.root = svrCfg.common.root;
        cfg.index = svrCfg.common.index;
        cfg.errorPage = svrCfg.common.errorPage;
        cfg.clientMaxBody = svrCfg.common.clientMaxBody;
        cfg.autoindex = (svrCfg.common.autoindex == "on") ? true : false;
        cfg.cgi = std::map< std::string, std::string >();
        cfg.redirect = std::pair< int, std::string >();

        addMethods(svrCfg.common.allowMethods, *itSvrName, "", cfg, r);
        addMethods(svrCfg.common.allowMethods, *itSvrName + ":" + port, "", cfg, r);

        addLocations(svrCfg, *itSvrName, r);
        addLocations(svrCfg, *itSvrName + ":" + port, r);
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
                addSvrToRouter(routers[ip + ":" + port], svrCfg, port);
            } else {
                std::map< std::string, IHandler* > hdlrs;
                hdlrs["GET"] = new GetHandler();
                hdlrs["POST"] = new UploadHandler();
                hdlrs["CGI"] = new CgiHandler();
                hdlrs["DELETE"] = new DeleteHandler();

                IRouter* r = new Router(hdlrs);
                addSvrToRouter(r, svrCfg, port);
                routers[ip + ":" + port] = r;
            }
        }
    }

    for (std::map< std::string, IRouter* >::iterator it = routers.begin(); it != routers.end(); it++) {
        std::cout << it->first << std::endl;
        it->second->printUrls();
    }

    return routers;
}

Router newRouter(std::vector< ServerConfig > svrCfgs, std::map< std::string, IHandler* > hdlrs) {
    Router r(hdlrs);
    for (std::vector< ServerConfig >::iterator it = svrCfgs.begin(); it != svrCfgs.end(); ++it) {
        ServerConfig svrCfg = *it;
        std::string port = "";
        addSvrToRouter(&r, svrCfg, port);
    }
    return r;
}
