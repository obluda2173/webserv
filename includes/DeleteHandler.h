#ifndef DELETEHANDLER_H
#define DELETEHANDLER_H

#include <sstream>
#include <fstream>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <filesystem>

#include "Router.h"
#include "Connection.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

class DeleteHandler : public IHandler {
  private:
    std::string _path;
    struct stat _pathStat;
    bool _validateDeleteRequest(Connection* conn, const HttpRequest& request, const RouteConfig& config);

  public:
    DeleteHandler();
    ~DeleteHandler();
    void handle(Connection* conn, const HttpRequest& req, const RouteConfig& config);
};

#endif // DELETEHANDLER_H