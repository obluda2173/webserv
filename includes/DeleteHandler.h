#ifndef DELETEHANDLER_H
#define DELETEHANDLER_H

#include "IHandler.h"
#include <dirent.h>
#include <sys/stat.h>

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
