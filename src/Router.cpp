#include "Router.h"
#include <algorithm>

ExecutionInfo Router::match(HttpRequest req) {
    std::string host = req.headers["host"];
    if (_svrs.find(host) == _svrs.end())
        host = _defaultSvr;

    std::string route = host + req.uri;
    if (!_uriToDirPath[route].empty()) {
        if (_uriToAllowedMethod[route].find(req.method) == _uriToAllowedMethod[route].end()) {
            return ExecutionInfo{"", "ERROR"};
        }
        return ExecutionInfo{_uriToDirPath[route] + req.uri, req.method};
    }

    std::vector<std::string> matches;
    std::set<std::string> _locs = _svrToLocs[req.headers["host"]];
    for (std::set<std::string>::iterator itLoc = _locs.begin(); itLoc != _locs.end(); itLoc++) {
        if (req.uri.compare(0, itLoc->length(), *itLoc) == 0)
            matches.push_back(*itLoc);
    }
    if (!matches.empty()) {
        route = req.headers["host"] + *std::max_element(matches.begin(), matches.end());
        if (!_uriToDirPath[route].empty()) {
            if (_uriToAllowedMethod[route].find(req.method) == _uriToAllowedMethod[route].end()) {
                return ExecutionInfo{"", "ERROR"};
            }
            return ExecutionInfo{_uriToDirPath[route] + req.uri, req.method};
        }
    }

    route = host;
    if (_uriToAllowedMethod[route].find(req.method) == _uriToAllowedMethod[route].end()) {
        return ExecutionInfo{"", "ERROR"};
    }
    return ExecutionInfo{_uriToDirPath[route] + req.uri, req.method};
}
void Router::add(std::string svrName, std::string prefix, std::string root, std::vector<std::string> allowedMethods) {
    if (_defaultSvr.empty())
        _defaultSvr = svrName;
    if (_svrs.find(svrName) == _svrs.end())
        _svrs.insert(svrName);
    if (_uriToDirPath.find(svrName + prefix) != _uriToDirPath.end())
        return;

    _uriToDirPath[svrName + prefix] = root;
    _uriToAllowedMethod[svrName + prefix] = std::set(allowedMethods.begin(), allowedMethods.end());
    if (allowedMethods.empty()) {
        _uriToAllowedMethod[svrName + prefix].insert("GET");
        _uriToAllowedMethod[svrName + prefix].insert("POST");
        _uriToAllowedMethod[svrName + prefix].insert("DELETE");
    }
    _svrToLocs[svrName].insert(prefix);
}
