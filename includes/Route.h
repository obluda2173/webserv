#ifndef ROUTE_H
#define ROUTE_H

#include "RouteConfig.h"
#include <map>

class IHandler;

struct Route {
    std::map< std::string, IHandler* > hdlrs;
    RouteConfig cfg;
};

#endif // ROUTE_H
