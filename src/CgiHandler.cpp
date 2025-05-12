#include "CgiHandler.h"

CgiHandler::CgiHandler() {}
CgiHandler::~CgiHandler() {}

std::vector<std::string> CgiHandler::_getCgiEnvironment(const HttpRequest& request) {
    // (void)request;
    std::vector<std::string> env;

    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_PROTOCOL=" + request.version);
    env.push_back("REQUEST_METHOD=" + request.method);
    env.push_back("SCRIPT_NAME=" + _path);
    env.push_back("PATH_INFO=" + request.uri);
    // env.push_back("QUERY_STRING=" + request.query); todo
    env.push_back("CONTENT_LENGTH=" + request.headers.at("content-length"));
    env.push_back("CONTENT_TYPE=" + request.headers.at("content-type"));

    return env;
}

std::string CgiHandler::_executeCgiScript() {
    int pipefd[2];
    pid_t pid;
    std::string output;

    if (pipe(pipefd) == -1) {
        return "";
    }
    if ((pid = fork()) == -1) {
        return "";
    }
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        // close pipefd[1]?
        // execve(...)
        exit(EXIT_FAILURE);
    } else {
        close(pipefd[1]);
        char buffer[4096];
        ssize_t count;
        
        while ((count = read(pipefd[0], buffer, sizeof(buffer)))) {
            if (count == -1) {
                break;
            }
            output.append(buffer, count);
        }
        waitpid(pid, NULL, 0);
        close(pipefd[0]);
    }
    return output;
}

void CgiHandler::_parseCgiOutput(const std::string& cgiOutput, HttpResponse& resp) {
    (void)cgiOutput;
    (void)resp;
}

void CgiHandler::handle(Connection* conn, const HttpRequest& request, const RouteConfig& config) {
    HttpResponse& resp = conn->_response;
    _path = normalizePath(config.root, request.uri);
    if (!validateRequest(resp, request, config, _path, _pathStat)) {
        conn->setState(Connection::SendResponse);
        return;
    }

    // environment
    // env = _getCgiEnvironment(*conn, request);

    // execute  script
    std::string cgiOutput = _executeCgiScript();
    if (cgiOutput.empty()) {
        setErrorResponse(resp, 500, "Internal Server Error", config);
        conn->setState(Connection::SendResponse);
        return;
    }

    // 4. Parse CGI output
    _parseCgiOutput(cgiOutput, resp);
    conn->setState(Connection::SendResponse);
}
