#ifndef DELETEHANDLER_H
#define DELETEHANDLER_H

#include <dirent.h>
#include <filesystem>

#include "handlerUtils.h"

class DeleteHandler : public IHandler {
  private:
    std::string _path;
    struct stat _pathStat;
    bool _deleteResource();

  public:
    DeleteHandler();
    ~DeleteHandler();
    void handle(Connection* conn, const HttpRequest& req, const RouteConfig& config);
};

#endif // DELETEHANDLER_H
