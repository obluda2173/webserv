#include "CgiHandler.h"

CgiHandler::CgiHandler() {}
CgiHandler::~CgiHandler() {}

std::vector<std::string> CgiHandler::_getCgiEnvironment(const HttpRequest& request) {
    std::vector<std::string> env;
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_PROTOCOL=" + request.version);
    env.push_back("REQUEST_METHOD=" + request.method);
    env.push_back("SCRIPT_NAME=" + _path);
    // env.push_back("PATH_INFO=" + request.uri); is it neccessary?
    env.push_back("QUERY_STRING=" + _query);
    // env.push_back("CONTENT_LENGTH=" + request.headers.at("content-length"));
    // env.push_back("CONTENT_TYPE=" + request.headers.at("content-type"));

    return env;
}

std::string CgiHandler::_executeCgiScript(std::vector<std::string> env) {
    int pipefd[2];
    pid_t pid;
    std::string output;

    const char* argv[] = {
        _interpreter.c_str(),
        _path.c_str(),
        nullptr
    };

    std::vector<const char*> env_ptrs;
        for (const auto& s : env) {
            env_ptrs.push_back(s.c_str());
        }
        env_ptrs.push_back(nullptr);
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
        execve(argv[0], const_cast<char* const*>(argv), const_cast<char* const*>(env_ptrs.data()));
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

std::string CgiHandler::_extractQuery(const std::string& uri) {
    size_t pos = uri.find("?");

    if (pos == std::string::npos) {
        return "";
    }
    std::string query = uri.substr(pos + 1);
    return query;
}

// can be deleted if make the getMimeType(const std::string& path) reusable
std::string CgiHandler::_findInterpreter(std::map<std::string, std::string> cgiMap) {
    size_t pos = _path.find(".") + 1;
    std::string ext;
    while (_path[pos] && _path[pos] != '/' && _path[pos] != '?') {
        ext += _path[pos];
        pos++;
    }
    std::map< std::string, std::string >::const_iterator it = cgiMap.find(ext);
    if (it != cgiMap.end()) {
        return it->second;
    }
    return "";
}

void CgiHandler::handle(Connection* conn, const HttpRequest& request, const RouteConfig& config) {
    HttpResponse& resp = conn->_response;
    _path = normalizePath(config.root, request.uri);
    _query = _extractQuery(request.uri);
    _interpreter = _findInterpreter(config.cgi);
    if (!validateRequest(resp, request, config, _path, _pathStat)) {
        conn->setState(Connection::SendResponse);
        return;
    }

    // environment
    std::vector<std::string> env = _getCgiEnvironment(request);

    // execute  script
    std::string cgiOutput = _executeCgiScript(env);
    if (cgiOutput.empty()) {
        setErrorResponse(resp, 500, "Internal Server Error", config);
        conn->setState(Connection::SendResponse);
        return;
    }

    
    setResponse(resp, 200, "OK", "text/php", cgiOutput.size(), new StringBodyProvider(cgiOutput));

    // 4. Parse CGI output
    // _parseCgiOutput(cgiOutput, resp);
    conn->setState(Connection::SendResponse);
}
