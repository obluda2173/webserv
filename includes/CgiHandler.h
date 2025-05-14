#ifndef CGIHANDLER_H
#define CGIHANDLER_H

#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#include "handlerUtils.h"

struct ExecParams {
  std::vector<const char*> argv;
  std::vector<const char*> env;
};

class CgiHandler : public IHandler {
  private:
    std::string _query;
    std::string _interpreter;
    std::string _path;
    struct stat _pathStat;
    std::vector<std::string> _envStorage;
    void _setCgiEnvironment(const HttpRequest& request);
    std::string _extractQuery(const std::string& uri);
    std::string _findInterpreter(std::map<std::string,std::string> cgiMap);
    void _prepareExecParams(const HttpRequest& request, ExecParams& params);
    void _parseCgiOutput(const std::string& cgiOutput, HttpResponse& resp);
    bool _validateAndPrepareContext(const HttpRequest& request, const RouteConfig& config, HttpResponse& resp);
    void _setupChildProcess(int pipefd[2]);
    void _setupParentProcess(Connection* conn, int pipefd[2], pid_t pid, const RouteConfig& cfg);

  public:
    CgiHandler();
    ~CgiHandler();
    void handle(Connection* conn, const HttpRequest& req, const RouteConfig& config);
    void handleCgiProcess(Connection* conn);
};

#endif // CGIHANDLER_H