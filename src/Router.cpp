#include "Router.h"
#include "HttpRequest.h"
#include <algorithm>
#include <map>
#include <unordered_map>

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
    if (_routeToRoutes.find(route) != _routeToRoutes.end())
        return _routeToRoutes[route];

    route = _matchLocations(req);
    if (_routeToRoutes.find(route) != _routeToRoutes.end())
        return _routeToRoutes[route];

    return _routeToRoutes[req.headers["host"]];
}

void Router::add(std::string svrName, std::string prefix, std::vector<std::string> allowedMethods, RouteConfig cfg) {
    if (_defaultSvr.empty())
        _defaultSvr = svrName;
    if (_svrs.find(svrName) == _svrs.end())
        _svrs.insert(svrName);
    if (_routeToRoutes.find(svrName + prefix) != _routeToRoutes.end())
        return;

    if (allowedMethods.size() == 0) {
        _routeToRoutes[svrName + prefix] = {
            {{"GET", _hdlrs["GET"]}, {"POST", _hdlrs["POST"]}, {"DELETE", _hdlrs["DELETE"]}}, cfg};
    } else {
        std::unordered_map<std::string, IHandler*> hdlrs;
        for (size_t i = 0; i < allowedMethods.size(); i++) {
            std::string method = allowedMethods[i];
            hdlrs[method] = _hdlrs[method];
        }
        _routeToRoutes[svrName + prefix] = {hdlrs, cfg};
    }

    _svrToLocs[svrName].insert(prefix);
}
