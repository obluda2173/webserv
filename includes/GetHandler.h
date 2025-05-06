#ifndef GETHANDLER_H
#define GETHANDLER_H

#include <sstream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <filesystem>

#include "Router.h"
#include "Connection.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

class GetHandler : public IHandler {
  private:
    std::string _path;
    struct stat _pathStat;
    static std::map<std::string, std::string> mimeTypes;
    bool _getValidation(Connection* conn, HttpRequest& request, RouteConfig& config);
    void _setErrorResponse(HttpResponse& resp, int code, const std::string& message);
    void _setGoodResponse(HttpResponse& resp, std::string mimeType, int statusCode, size_t fileSize, IBodyProvider* bodyProvider);
    std::string _normalizePath(const std::string& root, const std::string& uri);
    std::string _getMimeType(const std::string& path);
    std::string _getDirectoryListing(const std::string& path, const std::string& uri);

  public:
    GetHandler();
    ~GetHandler();
    void handle(Connection* conn, HttpRequest& req, RouteConfig& config);
};

#endif // GETHANDLER_H