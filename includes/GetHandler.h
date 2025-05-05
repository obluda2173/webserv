#ifndef GETHANDLER_H
#define GETHANDLER_H

#include <sys/stat.h>

#include "Router.h"
#include "HttpRequest.h"
#include "responseStructure.h"

class GetHandler : public IHandler {
  private:
    void _generateErrorMsg(HttpResponse& httpResponse, std::string errorCodeToPut, std::string errorMessageToPut);
  
  public:
    GetHandler();
    ~GetHandler();
    HttpResponse handle(Connection* conn, HttpRequest& req, RouteConfig& config);
};

#endif // GETHANDLER_H