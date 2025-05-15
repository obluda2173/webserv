#ifndef UPLOADHANDLER_H
#define UPLOADHANDLER_H

#include "Connection.h"
#include "HttpRequest.h"
#include "IHandler.h"
#include "RouteConfig.h"
#include <set>

class UploadHandler : public IHandler {
  private:
    std::set< std::string > _activeUploadPaths;
    bool _validation(Connection* conn, const HttpRequest& req, const RouteConfig& cfg);
    void _initUploadCxt(Connection* conn, const HttpRequest& req, const RouteConfig& cfg);
    bool _validateContentLength(Connection* conn, const RouteConfig& cfg);
    bool _validateFile(Connection* conn, const HttpRequest& req, const RouteConfig& cfg);
    bool _validateDir(Connection* conn, const HttpRequest& req, const RouteConfig& cfg);
    bool _validateNotActive(Connection* conn, const HttpRequest& req, const RouteConfig& cfg);

  public:
    ~UploadHandler();
    void uploadNewContent(Connection* conn);
    virtual void handle(Connection* conn, const HttpRequest& req, const RouteConfig& cfg);
};

#endif // UPLOADHANDLER_H
