#include "Router.h"

void addSvrToRouter(Router& r, ServerConfig svrCfg) {
    std::vector<std::string> srvNames = svrCfg.serverNames;
    for (std::vector<std::string>::iterator itSvrName = srvNames.begin(); itSvrName != srvNames.end(); itSvrName++) {
        RouteConfig cfg = {svrCfg.common.root, svrCfg.common.index, svrCfg.common.errorPage,
                           svrCfg.common.clientMaxBody, svrCfg.common.autoindex};
        r.add(*itSvrName, "", svrCfg.common.allowMethods, cfg);
        for (std::vector<LocationConfig>::iterator itLoc = svrCfg.locations.begin(); itLoc != svrCfg.locations.end();
             ++itLoc) {
            RouteConfig cfg = {itLoc->common.root, itLoc->common.index, itLoc->common.errorPage,
                               itLoc->common.clientMaxBody, itLoc->common.autoindex};
            r.add(*itSvrName, itLoc->prefix, itLoc->common.allowMethods, cfg);
        }
    }
}

Router newRouter(std::vector<ServerConfig> svrCfgs, IHandler* getHdlr, IHandler* postHdlr, IHandler* delHdlr) {
    Router r(getHdlr, postHdlr, delHdlr);
    for (std::vector<ServerConfig>::iterator it = svrCfgs.begin(); it != svrCfgs.end(); ++it) {
        ServerConfig svrCfg = *it;
        addSvrToRouter(r, svrCfg);
    }
    return r;
}
