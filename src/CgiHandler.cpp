#include "CgiHandler.h"
#include "handlerUtils.h"

CgiHandler::CgiHandler() {}
CgiHandler::~CgiHandler() {}

void CgiHandler::handle(Connection* conn, const HttpRequest& request, const RouteConfig& config) {
    while (true) {
        HttpResponse& resp = conn->_response;
        CgiContext& ctx = conn->cgiCtx;
        int status = 0;

        switch (ctx.state) {
        case CgiContext::Forking: {
            if (!_validateAndPrepareContext(request, config, conn)) {
                conn->setState(Connection::SendResponse);
                return;
            }
    
            int pipeStdin[2];
            int pipeStdout[2];
            if (pipe(pipeStdin) == -1 || pipe(pipeStdout) == -1) {
                setErrorResponse(resp, 500, config);
                conn->setState(Connection::SendResponse);
                return;
            }
    
            pid_t pid = fork();
            if (pid == -1) {
                close(pipeStdin[0]);
                close(pipeStdin[1]);
                close(pipeStdout[0]);
                close(pipeStdout[1]);
                setErrorResponse(resp, 500, config);
                conn->setState(Connection::SendResponse);
                return;
            }
    
            if (pid == 0) {
                _setupChildProcess(pipeStdin, pipeStdout, conn, request, config);
                execve(_execParams.argv[0], const_cast< char* const* >(_execParams.argv.data()),
                    const_cast< char* const* >(_execParams.env.data()));
                exit(EXIT_FAILURE);
            } else {
                _setupParentProcess(conn, pipeStdin, pipeStdout, pid, config);
                ctx.state = CgiContext::WritingStdin;
                break;
            }
            return;
        }
    
        case CgiContext::WritingStdin: {
            ssize_t bytesWritten = write(ctx.cgiPipeStdin,
                conn->_tempBody.data(), conn->_tempBody.size());
            if (bytesWritten == -1 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
                setErrorResponse(conn->_response, 500, ctx.cgiRouteConfig);
                conn->setState(Connection::SendResponse);
                return;
            }
            if (conn->_bodyFinished) {
                close(ctx.cgiPipeStdin);
                ctx.state = CgiContext::ReadingStdout;
                conn->setState(Connection::HandlingCgi);
                return;
            }
            break;
        }
    
        case CgiContext::ReadingStdout: {
            pid_t result = waitpid(ctx.cgiPid, &status, WNOHANG);
            if (result > 0) {
                ctx.state = CgiContext::Exited;
                break;
            } else if (result == -1) {
                setErrorResponse(conn->_response, 500, ctx.cgiRouteConfig);
                conn->setState(Connection::SendResponse);
                return;
            }
            if (!_readPipeData(ctx, false)) {
                setErrorResponse(conn->_response, 500, ctx.cgiRouteConfig);
                conn->setState(Connection::SendResponse);
                return;
            }
            break;
        }
    
        case CgiContext::Exited:
            _handleProcessExit(conn, ctx, status);
            return;
        }
    }
}

