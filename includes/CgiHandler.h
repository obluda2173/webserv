#ifndef CGIHANDLER_H
#define CGIHANDLER_H

#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#include "handlerUtils.h"

class CgiHandler : public IHandler {
  private:
    std::string _query;
    std::string _interpreter;
    std::string _path;
    struct stat _pathStat;
    std::vector<std::string> _getCgiEnvironment(const HttpRequest& request);
    std::string _executeCgiScript(std::vector<std::string> env);
    std::string _extractQuery(const std::string& uri);
    std::string _findInterpreter(std::map<std::string,std::string> cgiMap);
    void _parseCgiOutput(const std::string& cgiOutput, HttpResponse& resp);

  public:
    CgiHandler();
    ~CgiHandler();
    void handle(Connection* conn, const HttpRequest& req, const RouteConfig& config);
};

#endif // CGIHANDLER_H