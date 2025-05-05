#ifndef GETHANDLER_H
#define GETHANDLER_H

#include <sstream>
#include <sys/stat.h>

#include "Router.h"
#include "Connection.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

class GetHandler : public IHandler {
  private:
    bool _validation(Connection* conn, HttpRequest& request, RouteConfig& config);
  
  public:
    GetHandler();
    ~GetHandler();
    void handle(Connection* conn, HttpRequest& req, RouteConfig& config);
};

#endif // GETHANDLER_H