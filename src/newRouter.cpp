#include "Router.h"

void addSvrToRouter(Router& r, ServerConfig svrCfg) {
    std::vector<std::string> srvNames = svrCfg.serverNames;
    for (std::vector<std::string>::iterator itSvrName = srvNames.begin(); itSvrName != srvNames.end(); itSvrName++) {
        r.add(*itSvrName, "", svrCfg.common.root, svrCfg.common.allowMethods);
        for (std::vector<LocationConfig>::iterator itLoc = svrCfg.locations.begin(); itLoc != svrCfg.locations.end(); ++itLoc) {
            r.add(*itSvrName, itLoc->prefix, itLoc->common.root, itLoc->common.allowMethods);
        }
    }
}

Router newRouter(std::vector<ServerConfig> svrCfgs) {
    Router r;
    for (std::vector<ServerConfig>::iterator it = svrCfgs.begin(); it != svrCfgs.end(); ++it) {
        ServerConfig svrCfg = *it;
        addSvrToRouter(r, svrCfg);
    }
    return r;
}
