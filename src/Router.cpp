#include "Router.h"
#include <algorithm>

ExecutionInfo Router::match(HttpRequest req) {
    std::string host = req.headers["host"];
    if (_svrs.find(host) == _svrs.end())
        host = _defaultSvr;

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

    route = host + "/";
    if (_routeAllowedMethods[route].find(req.method) == _routeAllowedMethods[route].end()) {
        return ExecutionInfo{"", "ERROR"};
    }
    return ExecutionInfo{_routes[route] + req.uri, "GET"};
}
void Router::add(std::string svrName, std::string prefix, std::string root, std::vector<std::string> allowedMethods) {
    // if (_routes.find(svrName + prefix) != _routes.end())
    //     return; // don't consider doubled route
    _routes[svrName + prefix] = root;
    _routeAllowedMethods[svrName + prefix] = std::set(allowedMethods.begin(), allowedMethods.end());
    if (allowedMethods.empty()) {
        _routeAllowedMethods[svrName + prefix].insert("GET");
        _routeAllowedMethods[svrName + prefix].insert("POST");
        _routeAllowedMethods[svrName + prefix].insert("DELETE");
    }
    _svrToLocs[svrName].push_back(prefix);
}
