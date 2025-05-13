#ifndef UPLOADHANDLER_H
#define UPLOADHANDLER_H

#include "Connection.h"
#include "HttpRequest.h"
#include "IHandler.h"
#include "RouteConfig.h"
#include <fstream>
#include <sstream>

class UploadHandler : public IHandler {

  public:
    void uploadNewContent(Connection* conn);
    virtual void handle(Connection* conn, const HttpRequest& req, const RouteConfig& cfg);
};

#endif // UPLOADHANDLER_H
