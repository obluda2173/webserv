#ifndef IHANDLER_H
#define IHANDLER_H

#include "Connection.h"
#include "RouteConfig.h"

class IHandler {
  public:
    virtual ~IHandler() {}
    virtual void handle(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) = 0;
};


#endif // IHANDLER_H
