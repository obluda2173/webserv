#ifndef ROUTECONFIG_H
#define ROUTECONFIG_H

#include <map>
#include <string>
#include <vector>

struct RouteConfig {
    std::string root;
    std::vector< std::string > index;
    std::map< int, std::string > errorPage;
    size_t clientMaxBody;
    bool autoindex;
    std::map< std::string, std::string > cgi;
    std::pair< int, std::string > redirect;
};

#endif // ROUTECONFIG_H
