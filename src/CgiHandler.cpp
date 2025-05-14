#include "CgiHandler.h"

CgiHandler::CgiHandler() {}
CgiHandler::~CgiHandler() {}

std::vector<std::string> CgiHandler::_getCgiEnvironment(const HttpRequest& request) {
    std::vector<std::string> env;
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_PROTOCOL=" + request.version);
    env.push_back("REQUEST_METHOD=" + request.method);
    env.push_back("SCRIPT_NAME=" + _path);
    env.push_back("QUERY_STRING=" + _query);
    if (request.headers.count("content-length")) {
        env.push_back("CONTENT_LENGTH=" + request.headers.at("content-length"));
    }
    if (request.headers.count("content-type")) {
        env.push_back("CONTENT_TYPE=" + request.headers.at("content-type"));
    }

    return env;
}

std::string CgiHandler::_executeCgiScript(std::vector<std::string> envStr) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        return "";
    }

    const pid_t pid = fork();
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        return "";
    }

    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        std::vector<const char*> av;
        if (_interpreter.empty()) {
            av.push_back(_path.c_str());
        } else {
            av.push_back(_interpreter.c_str());
            av.push_back(_path.c_str());
        }
        av.push_back(nullptr);

        std::vector<const char*> envChr;
        for (size_t i = 0; i < envStr.size(); i++) {
            envChr.push_back(envStr[i].c_str());
        }
        envChr.push_back(nullptr);

        execve(av[0], const_cast<char* const*>(av.data()), const_cast<char* const*>(envChr.data()));
        exit(EXIT_FAILURE);
    } 
    close(pipefd[1]);
    std::string output;
    char buffer[4096];
    ssize_t count;

    while ((count = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
        output.append(buffer, count);
    }

    close(pipefd[0]);
    waitpid(pid, NULL, 0);
    return output;
}

void CgiHandler::_parseCgiOutput(const std::string& cgiOutput, HttpResponse& resp) {
    (void)cgiOutput;
    (void)resp;
}

std::string CgiHandler::_extractQuery(const std::string& uri) {
    const size_t pos = uri.find('?');
    return (pos != std::string::npos) ? uri.substr(pos + 1) : "";
}

// can be deleted if make the getMimeType(const std::string& path) reusable
std::string CgiHandler::_findInterpreter(std::map<std::string, std::string> cgiMap) {
    size_t dot = _path.rfind('.');
    if (dot == std::string::npos) {
        return "";
    }

    size_t end = _path.find_first_of("/?#", dot);
    std::string ext = _path.substr(dot + 1, end - dot - 1);
    
    std::map<std::string, std::string>::const_iterator it = cgiMap.find(ext);
    return it != cgiMap.end() ? it->second : "";
}

void CgiHandler::handle(Connection* conn, const HttpRequest& request, const RouteConfig& config) {
    HttpResponse& resp = conn->_response;
    _path = normalizePath(config.root, request.uri);
    _query = _extractQuery(request.uri);
    _interpreter = _findInterpreter(config.cgi);

    if (_interpreter.empty()) {
        setErrorResponse(resp, 403, "Forbidden", config);
        conn->setState(Connection::SendResponse);
        return;
    }

    if (!validateRequest(resp, request, config, _path, _pathStat)) {
        conn->setState(Connection::SendResponse);
        return;
    }

    std::vector<std::string> env = _getCgiEnvironment(request);
    std::string cgiOutput = _executeCgiScript(env);

    if (cgiOutput.empty()) {
        setErrorResponse(resp, 500, "Internal Server Error", config);
    } else {
        // _parseCgiOutput(cgiOutput, resp);
        setResponse(resp, 200, "OK", "text/php", cgiOutput.size(), new StringBodyProvider(cgiOutput));
    }

    conn->setState(Connection::SendResponse);
}
