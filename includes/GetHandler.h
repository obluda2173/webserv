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

#define DEFAULT_MIME_TYPE "application/octet-stream"
#define DEFAULT_HTTP_VERSION "HTTP/1.1"
#define DEFAULT_CONTENT_LANGUAGE "en-US"
#define MAX_PATH_LENGTH 4096

class GetHandler : public IHandler {
  private:
    std::string _path;
    struct stat _pathStat;
    bool _isInvalidHeader(const HttpRequest& req) const;
    bool _isValidPath() const;
    bool _isAccessible() const;
    bool _isValidFileType() const;
    bool _validateGetRequest(HttpResponse& resp, const HttpRequest& request, const RouteConfig& config);
    void _setErrorResponse(HttpResponse& resp, int code, const std::string& message, const RouteConfig& config);
    void _setResponse(HttpResponse& resp, int statusCode, const std::string& statusMessage, const std::string& contentType, size_t contentLength, IBodyProvider* bodyProvider);
    int _hexToInt(char c) const;
    std::string _decodePercent(const std::string& str) const;
    std::string _normalizePath(const std::string& root, const std::string& uri) const;
    std::string _getMimeType(const std::string& path) const;
    bool _getDirectoryListing(const std::string& path, const std::string& uri, std::string& outListing);
    
  public:
    static std::map<std::string, std::string> mimeTypes;
    GetHandler();
    ~GetHandler();
    void handle(Connection* conn, const HttpRequest& req, const RouteConfig& config);
};

#endif // GETHANDLER_H