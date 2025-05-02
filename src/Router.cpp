#include "Router.h"
#include "HttpRequest.h"
#include <algorithm>
#include <map>

std::string Router::_matchLocations(HttpRequest req) {
    std::string route = "";
    std::vector<std::string> matches;
    std::set<std::string> _locs = _svrToLocs[req.headers["host"]];
    for (std::set<std::string>::iterator itLoc = _locs.begin(); itLoc != _locs.end(); itLoc++) {
        if (req.uri.compare(0, itLoc->length(), *itLoc) == 0)
            matches.push_back(*itLoc);
    }
    if (!matches.empty())
        route = req.headers["host"] + *std::max_element(matches.begin(), matches.end());
    return route;
}

Route Router::match(HttpRequest req) {
    if (_svrs.find(req.headers["host"]) == _svrs.end())
        req.headers["host"] = _defaultSvr;

    std::string route = req.headers["host"] + req.uri;
    if (!_routeToRoot[route].empty())
        return _routeToRoutes[req.headers["host"]];

    route = _matchLocations(req);
    if (!_routeToRoot[route].empty())
        return _routeToRoutes[req.headers["host"]];

    return _routeToRoutes[req.headers["host"]];
}

void Router::add(std::string svrName, std::string prefix, std::string root, std::vector<std::string> allowedMethods) {
    if (_defaultSvr.empty())
        _defaultSvr = svrName;
    if (_svrs.find(svrName) == _svrs.end())
        _svrs.insert(svrName);
    if (_routeToRoot.find(svrName + prefix) != _routeToRoot.end())
        return;

    _routeToRoot[svrName + prefix] = root;

    if (allowedMethods.size() == 1) {
        _routeToRoutes[svrName + prefix] = {{{"GET", _hdlrs["GET"]}}, {root}};
    }

    if (allowedMethods.size() == 0) {
        _routeToRoutes[svrName + prefix] = {{{"GET", _hdlrs["GET"]}, {"POST", _hdlrs["POST"]}, {"DELETE", _hdlrs["DELETE"]}}, {root}};
    }

    _svrToLocs[svrName].insert(prefix);
}
