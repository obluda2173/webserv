#ifndef GETHANDLER_H
#define GETHANDLER_H

#include "HttpResponse.h"
#include "IHandler.h"
#include <dirent.h>
#include <sys/stat.h>

class GetHandler : public IHandler {
  private:
    std::string _path;
    struct stat _pathStat;
    bool _getDirectoryListing(const std::string& path, const std::string& uri, std::string& outListing);
    void _serveRegFile(Connection& conn, HttpResponse& resp);
    void _serveIndexFile(Connection& conn, const RouteConfig& config, HttpResponse& resp);
    void _serveDirectoryListing(Connection& conn, const HttpRequest& request, HttpResponse& resp);

  public:
    GetHandler();
    ~GetHandler();
    void handle(Connection* conn, const HttpRequest& req, const RouteConfig& config);
};

#endif // GETHANDLER_H
