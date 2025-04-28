// #include "Router.h"

// Router::Router(std::map<std::string, std::vector<std::string>> svrNameToLocPrefixes)
//     : _svrNameToLocPrefixes(svrNameToLocPrefixes) {}

// std::string Router::match(const HttpRequest& request) {
//     std::string host = request.headers.find("host")->second;
//     std::vector locPrefixes = _svrNameToLocPrefixes.at(host);

//     std::string bestLoc;
//     size_t bestLen = 0;
//     const std::string& path = request.uri; // e.g., "/images/cat.png"
//     for (std::vector<std::string>::const_iterator locPrefix = locPrefixes.begin(); locPrefix != locPrefixes.end();
//          ++locPrefix) {
//         if (path.compare(0, locPrefix->size(), *locPrefix) == 0) {
//             if (locPrefix->size() > bestLen) {
//                 bestLen = locPrefix->size();
//                 bestLoc = *locPrefix;
//             }
//         }
//     }

//     return bestLoc;
// }

// #include "Router.h"
// #include "ConfigStructure.h"

// Router newRouter(std::vector<ServerConfig> svrCfgs) {
//     Router r;
//     for (ServerConfig svrCfg : svrCfgs) {
//         r.addSvrCfg(svrCfg);
//         for (LocationConfig locCfg : svrCfg.locations) {
//             r.addLocCfg(locCfg);
//         }
//     }
//     return r;
// }

// Router::addSvrCfg() {
//     // 1. go through svrCfg and
//     //    1. construct the url
//     //    2. construct Handler
//     //    3. add Handler to dataStructure
// }
