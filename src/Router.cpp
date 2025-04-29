#include "Router.h"
#include "ConfigStructure.h"
#include <algorithm>

GetHandler Router::match(HttpRequest req) {
    std::string url = req.headers["host"] + req.uri;
    if (!_svrMap[url].empty())
        return GetHandler(_svrMap[url] + req.uri);

    std::vector<std::string> matches;
    std::vector<std::string> _locs = _allLocs[req.headers["host"]];
    for (size_t i = 0; i < _locs.size(); i++) {
        if (req.uri.compare(0, _locs[i].length(), _locs[i]) == 0)
            matches.push_back(_locs[i]);
    }
    if (!matches.empty()) {
        url = req.headers["host"] + *std::max_element(matches.begin(), matches.end());
        if (!_svrMap[url].empty())
            return GetHandler(_svrMap[url] + req.uri);
    }

    url = req.headers["host"] + "/";
    if (!_svrMap[url].empty())
        return GetHandler(_svrMap[url] + req.uri);

    req.headers["host"] = _svrMap["default"];
    return match(req);
}

void addSvrToRouter(Router& r, ServerConfig svrCfg) {
    std::vector<std::string> srvNames = svrCfg.serverNames;
    for (std::vector<std::string>::iterator itSvrName = srvNames.begin(); itSvrName != srvNames.end(); itSvrName++) {
        r.add(*itSvrName, "/", svrCfg.common.root);
        for (std::vector<LocationConfig>::iterator itLoc = svrCfg.locations.begin(); itLoc != svrCfg.locations.end();
             ++itLoc) {
            r.add(*itSvrName, itLoc->prefix, itLoc->common.root);
        }
    }
}

Router newRouter(std::vector<ServerConfig> svrCfgs) {
    Router r;
    for (std::vector<ServerConfig>::iterator it = svrCfgs.begin(); it != svrCfgs.end(); ++it) {
        ServerConfig svrCfg = *it;
        if (it == svrCfgs.begin())
            r.add("default", "", svrCfg.serverNames[0]);
        addSvrToRouter(r, svrCfg);
    }
    return r;
}
