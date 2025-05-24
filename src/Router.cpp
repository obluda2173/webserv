#include "Router.h"
#include "HttpRequest.h"
#include <algorithm>
#include <map>

std::string Router::_matchLocations(HttpRequest req) {
    std::string route = "";
    std::vector< std::string > matches;
    std::set< std::string > _locs = _svrToLocs[req.headers["host"]];
    for (std::set< std::string >::iterator itLoc = _locs.begin(); itLoc != _locs.end(); itLoc++) {
        if (req.uri.compare(0, itLoc->length(), *itLoc) == 0)
            matches.push_back(*itLoc);
    }
    if (!matches.empty())
        route = req.headers["host"] + *std::max_element(matches.begin(), matches.end());
    return route;
}

Route Router::match(HttpRequest req) {
    // when host is not known, make it defaultSvr
    if (_svrs.find(req.headers["host"]) == _svrs.end())
        req.headers["host"] = _defaultSvr;

    std::string url = req.headers["host"] + req.uri;
    if (_urlToRoutes.find(url) != _urlToRoutes.end())
        return _urlToRoutes[url];

    url = _matchLocations(req);
    if (_urlToRoutes.find(url) != _urlToRoutes.end())
        return _urlToRoutes[url];

    return _urlToRoutes[req.headers["host"]];
}

void Router::add(std::string svrName, std::string prefix, std::string method, RouteConfig cfg) {
    if (_defaultSvr.empty())
        _defaultSvr = svrName;
    if (_svrs.find(svrName) == _svrs.end())
        _svrs.insert(svrName);

    if (_svrKnownPrefixPlusMethod[svrName].find(prefix + method) != _svrKnownPrefixPlusMethod[svrName].end())
        return;

    _urlToRoutes[svrName + prefix].cfg = cfg;
    _urlToRoutes[svrName + prefix].hdlrs[method] = _hdlrs[method];
    _svrToLocs[svrName].insert(prefix);
    _svrKnownPrefixPlusMethod[svrName].insert(prefix + method);
}
