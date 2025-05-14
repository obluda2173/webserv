#ifndef UPLOADHANDLER_H
#define UPLOADHANDLER_H

#include "Connection.h"
#include "HttpRequest.h"
#include "IHandler.h"
#include "RouteConfig.h"

class UploadHandler : public IHandler {
  private:
    bool _validation(Connection* conn, const RouteConfig& cfg);
    void _initUploadCxt(Connection* conn, const HttpRequest& req, const RouteConfig& cfg);
    bool _validateContentLength(Connection* conn, const RouteConfig& cfg);

  public:
    ~UploadHandler();
    void uploadNewContent(Connection* conn);
    virtual void handle(Connection* conn, const HttpRequest& req, const RouteConfig& cfg);
};

#endif // UPLOADHANDLER_H
