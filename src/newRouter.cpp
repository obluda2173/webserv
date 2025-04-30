#include "Router.h"

void addSvrToRouter(Router& r, ServerConfig svrCfg) {
    std::vector<std::string> srvNames = svrCfg.serverNames;
    for (std::vector<std::string>::iterator itSvrName = srvNames.begin(); itSvrName != srvNames.end(); itSvrName++) {
        r.add(*itSvrName, "/", svrCfg.common.root,
              svrCfg.common.allowMethods); // TODO: need to put in a configuration without root on server_level
        for (std::vector<LocationConfig>::iterator itLoc = svrCfg.locations.begin(); itLoc != svrCfg.locations.end();
             ++itLoc) {
            r.add(*itSvrName, itLoc->prefix, itLoc->common.root, itLoc->common.allowMethods);
        }
    }
}

Router newRouter(std::vector<ServerConfig> svrCfgs) {
    Router r;
    for (std::vector<ServerConfig>::iterator it = svrCfgs.begin(); it != svrCfgs.end(); ++it) {
        ServerConfig svrCfg = *it;
        if (it == svrCfgs.begin())
            r.add("default", "", svrCfg.serverNames[0], svrCfg.common.allowMethods);
        addSvrToRouter(r, svrCfg);
    }
    return r;
}

Router newRouterTest() {
    std::map<std::string, std::vector<std::string>> svrToLocs;
    std::map<std::string, std::string> routes;
    std::map<std::string, std::set<std::string>> routeAllowedMethods;

    svrToLocs = {
        {"example.com", {"/", "/images/", "/css/scripts/", "/css/", "/css/styles/"}},
        {"test.com", {"/", "/css/", "/js/", "/images/"}},
        {"www.test.com", {"/", "/css/", "/js/", "/images/"}},
        {"test2.com", {"/"}},
        {"test3.com", {"/"}},

    };

    routes = {
        {"default", "example.com"},
        {"example.com/", "/var/www/html"},
        {"example.com/images/", "/data"},
        {"example.com/css/scripts/", "/data/scripts"},
        {"example.com/css/", "/data/static"},
        {"example.com/css/styles/", "/data/extra"},

        {"test.com/", "/var/www/secure"},
        {"test.com/css/", "/data/static"},
        {"test.com/js/", "/data/scripts"},
        {"test.com/images/", "/data2"},
        {"www.test.com/", "/var/www/secure"},
        {"www.test.com/cs/", "/data/static"},
        {"www.test.com/js/", "/data/scripts"},
        {"www.test.com/images/", "/data2"},

        {"test2.com/", "/usr/share/nginx/html"},

        {"test3.com/", "/test3/www/html"},

    };

    routeAllowedMethods = {
        {"example.com/", {"GET"}},
        {"example.com/images/", {"GET", "POST", "DELETE"}},
        {"example.com/css/scripts/", {"GET", "POST", "DELETE"}},
        {"example.com/css/", {"GET", "POST", "DELETE"}},
        {"example.com/css/styles/", {"GET", "POST", "DELETE"}},

        {"test.com/", {"GET", "POST", "DELETE"}},
        {"test.com/css/", {"GET", "POST", "DELETE"}},
        {"test.com/js/", {"GET"}},
        {"test.com/images/", {"GET", "POST", "DELETE"}},
        {"www.test.com/", {"GET", "POST", "DELETE"}},
        {"www.test.com/css/", {"GET", "POST", "DELETE"}},
        {"www.test.com/js/", {"GET", "POST", "DELETE"}},
        {"www.test.com/images/", {"GET", "POST", "DELETE"}},

        {"test2.com/", {"GET"}},

        {"test3.com/", {"GET", "POST", "DELETE"}},
    };

    return Router(routes, routeAllowedMethods, svrToLocs);
}
