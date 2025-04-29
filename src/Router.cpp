#include "Router.h"
#include "ConfigStructure.h"
#include <algorithm>

ExecutionInfo Router::match(HttpRequest req) {
    std::string route = req.headers["host"] + req.uri;
    if (!_routes[route].empty()) {
        if (_routeAllowedMethods[route].find(req.method) == _routeAllowedMethods[route].end()) {
            return ExecutionInfo{"", "ERROR"};
        }
        return ExecutionInfo{_routes[route] + req.uri, req.method};
    }

    std::vector<std::string> matches;
    std::vector<std::string> _locs = _svrToLocs[req.headers["host"]];
    for (size_t i = 0; i < _locs.size(); i++) {
        if (req.uri.compare(0, _locs[i].length(), _locs[i]) == 0)
            matches.push_back(_locs[i]);
    }
    if (!matches.empty()) {
        route = req.headers["host"] + *std::max_element(matches.begin(), matches.end());
        if (!_routes[route].empty()) {
            if (_routeAllowedMethods[route].find(req.method) == _routeAllowedMethods[route].end()) {
                return ExecutionInfo{"", "ERROR"};
            }
            return ExecutionInfo{_routes[route] + req.uri, "GET"};
        }
    }

    route = req.headers["host"] + "/";
    if (!_routes[route].empty())
        return ExecutionInfo{_routes[route] + req.uri, "GET"};

    req.headers["host"] = _routes["default"];
    return match(req);
}

void addSvrToRouter(Router& r, ServerConfig svrCfg) {
    std::vector<std::string> srvNames = svrCfg.serverNames;
    for (std::vector<std::string>::iterator itSvrName = srvNames.begin(); itSvrName != srvNames.end(); itSvrName++) {
        r.add(*itSvrName, "/", svrCfg.common.root, svrCfg.common.allowMethods);
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
