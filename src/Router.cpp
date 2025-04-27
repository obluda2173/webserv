#include "Router.h"

Router::Router(const std::vector<ServerConfig>& servers) : _servers(servers) {
    for (size_t i = 0; i < _servers.size(); ++i) {
        for (std::map<std::string, int>::const_iterator it = _servers[i].listen.begin();
             it != _servers[i].listen.end(); ++it) 
        {
            // const std::string& ip = it->first;   // key (IP/host)
            int port = it->second;                  // value (port)
            _byPort[port].push_back(&_servers[i]);
        }
    }
}

std::pair<const ServerConfig*, const LocationConfig*> Router::match(const HttpRequest& request, int destPost) {
    const std::string& host = request.headers.find("host")->second;

    std::map<int, std::vector<const ServerConfig*>>::iterator it = _byPort.find(destPost);
    if (it == _byPort.end()) {
        throw std::runtime_error("No server found for the given port");
    }

    const std::vector<const ServerConfig*>& candidates = it->second;

    const ServerConfig* chosen = NULL;
    for (const ServerConfig* srv : candidates) {
        for (std::vector<std::string>::const_iterator hn = srv->serverNames.begin(); hn != srv->serverNames.end(); ++hn) {
            if (*hn == host) {
                chosen = srv;
                break;
            }
        }
        if (chosen) {
            break;
        }
    }
    if (!chosen) {
        chosen = candidates.front();
    }

    const LocationConfig* bestLoc = NULL;
    size_t bestLen = 0;
    const std::string& path = request.uri;  // e.g., "/images/cat.png"

    for (std::vector<LocationConfig>::const_iterator L = chosen->locations.begin();
         L != chosen->locations.end(); ++L) {
        const std::string& prefix = L->prefix; // e.g. "/images/"
        if (path.compare(0, prefix.size(), prefix) == 0) {
            if (prefix.size() > bestLen) {
                bestLen = prefix.size();
                bestLoc = &*L;
            }
        }
    }
    // static const LocationConfig rootLoc = { "/", chosen->defaultConfig };
    // if (!bestLoc) bestLoc = &rootLoc;

    return std::make_pair(chosen, bestLoc);
}
