#include "Router.h"
#include "HttpRequest.h"
#include <algorithm>

ExecutionInfo Router::_checkAllowedMethods(std::string route, HttpRequest req) {
    if (_routeToAllowedMethod[route].find(req.method) == _routeToAllowedMethod[route].end()) {
        return ExecutionInfo{"", "ERROR"};
    }
    return ExecutionInfo{_routeToDirPath[route] + req.uri, req.method};
}

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

ExecutionInfo Router::match(HttpRequest req) {
    std::string host = req.headers["host"];
    if (_svrs.find(host) == _svrs.end())
        host = _defaultSvr;

    std::string route = host + req.uri;
    if (!_routeToDirPath[route].empty())
        return _checkAllowedMethods(route, req);

    route = _matchLocations(req);
    if (!_routeToDirPath[route].empty())
        return _checkAllowedMethods(route, req);

    route = host;
    return _checkAllowedMethods(route, req);
}
void Router::add(std::string svrName, std::string prefix, std::string root, std::vector<std::string> allowedMethods) {
    if (_defaultSvr.empty())
        _defaultSvr = svrName;
    if (_svrs.find(svrName) == _svrs.end())
        _svrs.insert(svrName);
    if (_routeToDirPath.find(svrName + prefix) != _routeToDirPath.end())
        return;

    _routeToDirPath[svrName + prefix] = root;
    _routeToAllowedMethod[svrName + prefix] = std::set(allowedMethods.begin(), allowedMethods.end());
    if (allowedMethods.empty()) {
        _routeToAllowedMethod[svrName + prefix].insert("GET");
        _routeToAllowedMethod[svrName + prefix].insert("POST");
        _routeToAllowedMethod[svrName + prefix].insert("DELETE");
    }
    _svrToLocs[svrName].insert(prefix);
}
