#include "Router.h"

void addSvrToRouter(Router& r, ServerConfig svrCfg, std::map<std::string, IHandler*> hdlrs) {
    std::vector<std::string> srvNames = svrCfg.serverNames;
    for (std::vector<std::string>::iterator itSvrName = srvNames.begin(); itSvrName != srvNames.end(); itSvrName++) {
        RouteConfig cfg = {svrCfg.common.root, svrCfg.common.index, svrCfg.common.errorPage,
                           svrCfg.common.clientMaxBody, svrCfg.common.autoindex};
        if (cfg.clientMaxBody == 0)
            cfg.clientMaxBody = oneMB;

        for (size_t i = 0; i < svrCfg.common.allowMethods.size(); i++) {
            std::string method = svrCfg.common.allowMethods[i];
            if (hdlrs.find(method) == hdlrs.end()) {
                std::cerr << "Method not present in handler" << std::endl;
                exit(1);
            }
            r.add(*itSvrName, "", method, hdlrs[method], cfg);
        }

        if (svrCfg.common.allowMethods.empty()) {
            r.add(*itSvrName, "", "GET", hdlrs["GET"], cfg);
            r.add(*itSvrName, "", "POST", hdlrs["POST"], cfg);
            r.add(*itSvrName, "", "DELETE", hdlrs["DELETE"], cfg);
        }

        std::set<std::string> presentLocs;
        for (std::vector<LocationConfig>::iterator itLoc = svrCfg.locations.begin(); itLoc != svrCfg.locations.end();
             ++itLoc) {
            if (presentLocs.find(itLoc->prefix) != presentLocs.end())
                continue;
            presentLocs.insert(itLoc->prefix);
            RouteConfig cfg = {itLoc->common.root, itLoc->common.index, itLoc->common.errorPage,
                               itLoc->common.clientMaxBody, itLoc->common.autoindex};

            if (cfg.clientMaxBody == 0)
                cfg.clientMaxBody = oneMB;

            for (size_t i = 0; i < itLoc->common.allowMethods.size(); i++) {
                std::string method = itLoc->common.allowMethods[i];
                if (hdlrs.find(method) == hdlrs.end()) {
                    std::cerr << "Method not present in handler" << std::endl;
                    exit(1);
                }
                r.add(*itSvrName, itLoc->prefix, method, hdlrs[method], cfg);
                // r.add(*itSvrName, "", method, hdlrs[method], cfg);
            }

            if (itLoc->common.allowMethods.empty()) {
                // r.add(*itSvrName, itLoc->prefix, itLoc->common.allowMethods, cfg);
                r.add(*itSvrName, itLoc->prefix, "GET", hdlrs["GET"], cfg);
                r.add(*itSvrName, itLoc->prefix, "POST", hdlrs["POST"], cfg);
                r.add(*itSvrName, itLoc->prefix, "DELETE", hdlrs["DELETE"], cfg);
            }
        }
    }
}

Router newRouter(std::vector<ServerConfig> svrCfgs, std::map<std::string, IHandler*> hdlrs) {
    Router r(hdlrs);
    for (std::vector<ServerConfig>::iterator it = svrCfgs.begin(); it != svrCfgs.end(); ++it) {
        ServerConfig svrCfg = *it;
        addSvrToRouter(r, svrCfg, hdlrs);
    }
    return r;
}
