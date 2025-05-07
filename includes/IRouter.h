#ifndef IROUTER_H
#define IROUTER_H

#include "Route.h"
#include "RouteConfig.h"
#include <string>

class IRouter {
  public:
    virtual ~IRouter() {}
    virtual void add(std::string svrName, std::string prefix, std::string method, RouteConfig cfg) = 0;
    virtual Route match(HttpRequest req) = 0;
};

#endif // IROUTER_H
