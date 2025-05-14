#include "CgiHandler.h"

CgiHandler::CgiHandler() {}
CgiHandler::~CgiHandler() {}

void CgiHandler::handle(Connection* conn, const HttpRequest& request, const RouteConfig& config) {
    HttpResponse& resp = conn->_response;

    if (!_validateAndPrepareContext(request, config, resp)) {
        conn->setState(Connection::SendResponse);
        return;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        setErrorResponse(resp, 500, "Internal Server Error", config);
        conn->setState(Connection::SendResponse);
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        setErrorResponse(resp, 500, "Internal Server Error", config);
        conn->setState(Connection::SendResponse);
        return;
    }

    if (pid == 0) {
        ExecParams params;
        _setupChildProcess(pipefd);
        _prepareExecParams(request, params);
        execve(params.argv[0], 
            const_cast<char* const*>(params.argv.data()),
            const_cast<char* const*>(params.env.data()));
        exit(EXIT_FAILURE);
    } else {
        _setupParentProcess(conn, pipefd, pid, config);
    }
}

void CgiHandler::handleCgiProcess(Connection* conn) {
    CgiContext& ctx = conn->cgiCtx;

    if (ctx.cgiPipeFd != -1) {
        char buffer[4096];
        ssize_t count = read(ctx.cgiPipeFd, buffer, sizeof(buffer));
        if (count > 0) {
            ctx.cgiOutput.append(buffer, count);
        } else if (count == 0) {
            close(ctx.cgiPipeFd);
            ctx.cgiPipeFd = -1;
        } else if (count == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
            close(ctx.cgiPipeFd);
            ctx.cgiPipeFd = -1;
        }
    }

    int status;
    pid_t result = waitpid(ctx.cgiPid, &status, WNOHANG);
    if (result == -1) {
        setErrorResponse(conn->_response, 500, "Internal Server Error", ctx.cgiRouteConfig);
        conn->setState(Connection::SendResponse);
    } else if (result > 0) {
        if (ctx.cgiPipeFd != -1) {
            while (true) {
                char buffer[4096];
                ssize_t n = read(ctx.cgiPipeFd, buffer, sizeof(buffer));
                if (n > 0) ctx.cgiOutput.append(buffer, n);
                else if (n == 0) break;
                else if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                else break;
            }
            close(ctx.cgiPipeFd);
            ctx.cgiPipeFd = -1;
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            if (ctx.cgiOutput.empty()) {
                setErrorResponse(conn->_response, 500, "Internal Server Error", ctx.cgiRouteConfig);
            } else {
                // _parseCgiOutput
                setResponse(conn->_response, 200, "OK", "text/php", ctx.cgiOutput.size(), new StringBodyProvider(ctx.cgiOutput));
            }
        } else {
            setErrorResponse(conn->_response, 502, "Bad Gateway", ctx.cgiRouteConfig);
        }
        conn->setState(Connection::SendResponse);
    }
}
