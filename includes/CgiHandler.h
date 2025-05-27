#ifndef CGIHANDLER_H
#define CGIHANDLER_H

#include <fcntl.h>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "utils.h"
#include "logging.h"
#include "IHandler.h"
#include "Connection.h"

std::string toUpper(const std::string& str);
void replace(std::string& str, char what, char with);
std::string trimWhiteSpace(const std::string& str);
std::string extractQuery(const std::string& uri);
std::string findInterpreter(std::map< std::string, std::string > cgiMap, const std::string& uri);
std::string getScriptName(const std::map<std::string, std::string>& cgiMap, const std::string& uri);
std::string getPathInfo(std::map< std::string, std::string > cgiMap, const std::string& uri);
std::string getServerPort(Connection* conn);
std::string getRemoteAddr(Connection* conn);

struct ExecParams {
    std::vector< const char* > argv;
    std::vector< const char* > env;
};

class CgiHandler : public IHandler {
  private:
    std::string _path;
    std::string _interpreter;
    ExecParams _execParams;
    struct stat _pathStat;
    std::vector< std::string > _envStorage;
    bool _validateAndPrepareContext(const HttpRequest& request, const RouteConfig& config, Connection* conn);
    void _setCgiEnvironment(const HttpRequest& request, const RouteConfig& config, Connection* conn);
    void _setupChildProcess(int pipeStdin[2], int pipeStdout[2], Connection* conn, const HttpRequest& request, const RouteConfig& config);
    void _setupParentProcess(Connection* conn, int pipeStdin[2], int pipeStdout[2], pid_t pid, const RouteConfig& config);
    void _cgiResponseSetup(const std::string& cgiOutput, HttpResponse& resp, RouteConfig& config);
    bool _writeToStdin(Connection* conn, CgiContext& ctx);
    bool _readPipeData(CgiContext& cgiCtx, bool drain);
    void _handleProcessExit(Connection* conn, CgiContext& ctx, int status);

  public:
    CgiHandler();
    ~CgiHandler();
    void handle(Connection* conn, const HttpRequest& req, const RouteConfig& config);
    void handleCgiProcess(Connection* conn);
};

#endif // CGIHANDLER_H
