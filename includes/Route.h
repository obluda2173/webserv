#ifndef ROUTE_H
#define ROUTE_H

#include "RouteConfig.h"
#include "IHandler.h"
#include <map>

struct Route {
    std::map<std::string, IHandler*> hdlrs;
    RouteConfig cfg;
};

#endif // ROUTE_H
