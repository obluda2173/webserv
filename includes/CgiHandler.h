#ifndef CGIHANDLER_H
#define CGIHANDLER_H

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include "Connection.h"
#include "IHandler.h"

enum class ProcessState { Running, Exited, Error };

struct ExecParams {
    std::vector< const char* > argv;
    std::vector< const char* > env;
};

class CgiHandler : public IHandler {
  private:
    std::string _query;
    std::string _interpreter;
    std::string _path;
    struct stat _pathStat;
    std::vector< std::string > _envStorage;
    std::string _extractQuery(const std::string& uri);
    std::string _findInterpreter(std::map< std::string, std::string > cgiMap);
    bool _validateAndPrepareContext(const HttpRequest& request, const RouteConfig& config, HttpResponse& resp);
    void _setCgiEnvironment(const HttpRequest& request);
    void _prepareExecParams(const HttpRequest& request, ExecParams& params);
    void _setupChildProcess(int pipefd[2]);
    void _setupParentProcess(Connection* conn, int pipefd[2], pid_t pid, const RouteConfig& cfg);
    void _parseCgiOutput(const std::string& cgiOutput, HttpResponse& resp);
    ProcessState _checkProcess(CgiContext& ctx, int& status);
    bool _readPipeData(CgiContext& cgiCtx, bool drain);
    void _handleProcessExit(Connection* conn, CgiContext& ctx, int status);

  public:
    CgiHandler();
    ~CgiHandler();
    void handle(Connection* conn, const HttpRequest& req, const RouteConfig& config);
    void handleCgiProcess(Connection* conn);
};

#endif // CGIHANDLER_H
