#ifndef ROUTECONFIG_H
#define ROUTECONFIG_H

#include <string>
#include <vector>
#include <map>

const size_t oneMB = 1 * 1024 * 1024;
const size_t oneKB = 1 * 1024;
struct RouteConfig {
    std::string root;
    std::vector<std::string> index;
    std::map<int, std::string> errorPage;
    size_t clientMaxBody;
    bool autoindex;
};

#endif // ROUTECONFIG_H
