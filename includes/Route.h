#ifndef ROUTE_H
#define ROUTE_H

#include "RouteConfig.h"
#include "IHandler.h"
#include <unordered_map>

struct Route {
    std::unordered_map<std::string, IHandler*> hdlrs;
    RouteConfig cfg;
};

#endif // ROUTE_H
