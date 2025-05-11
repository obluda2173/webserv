#ifndef CGIHANDLER_H
#define CGIHANDLER_H

#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#include "handlerUtils.h"

class CgiHandler : public IHandler {
  private:
    std::string _path;
    struct stat _pathStat;
    void _setCgiEnvironment(Connection& conn, const HttpRequest& request);
    std::string _executeCgiScript();
    void _parseCgiOutput(const std::string& cgiOutput, HttpResponse& resp);

  public:
    CgiHandler();
    ~CgiHandler();
    void handle(Connection* conn, const HttpRequest& req, const RouteConfig& config);
};

#endif // CGIHANDLER_H