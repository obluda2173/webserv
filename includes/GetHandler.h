#ifndef GETHANDLER_H
#define GETHANDLER_H

#include <sstream>
#include <fstream>
#include <dirent.h>
#include <filesystem>

#include "handlerUtils.h"

class GetHandler : public IHandler {
  private:
    std::string _path;
    struct stat _pathStat;
    bool _isInvalidHeader(const HttpRequest& req) const;
    bool _isValidPath() const;
    bool _isAccessible() const;
    bool _isValidFileType() const;
    bool _getDirectoryListing(const std::string& path, const std::string& uri, std::string& outListing);
    
  public:
    GetHandler();
    ~GetHandler();
    void handle(Connection* conn, const HttpRequest& req, const RouteConfig& config);
};

#endif // GETHANDLER_H