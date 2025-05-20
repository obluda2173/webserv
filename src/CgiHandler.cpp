#include "CgiHandler.h"
#include "handlerUtils.h"

CgiHandler::CgiHandler() {}
CgiHandler::~CgiHandler() {}

void CgiHandler::handle(Connection* conn, const HttpRequest& request, const RouteConfig& config) {
    HttpResponse& resp = conn->_response;

    if (!_validateAndPrepareContext(request, config, resp)) {
        conn->setState(Connection::SendResponse);
        return;
    }

    int pipeStdin[2];
    int pipeStdout[2];
    if (pipe(pipeStdin) == -1 || pipe(pipeStdout) == -1) {
        setErrorResponse(resp, 500, "Internal Server Error", config);
        conn->setState(Connection::SendResponse);
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        close(pipeStdin[0]);
        close(pipeStdin[1]);
        close(pipeStdout[0]);
        close(pipeStdout[1]);
        setErrorResponse(resp, 500, "Internal Server Error", config);
        conn->setState(Connection::SendResponse);
        return;
    }

    if (pid == 0) {
        ExecParams params;
        _setupChildProcess(pipeStdin, pipeStdout);
        _prepareExecParams(request, params);
        execve(params.argv[0], const_cast< char* const* >(params.argv.data()),
               const_cast< char* const* >(params.env.data()));
        exit(EXIT_FAILURE);
    } else {
        _setupParentProcess(conn, pipeStdin, pipeStdout, pid, config);
    }
}

void CgiHandler::handleCgiProcess(Connection* conn) {
    CgiContext& ctx = conn->cgiCtx;
    int status = 0;

    switch (_checkProcess(ctx, status)) {
    case ProcessState::Exited:
        _handleProcessExit(conn, ctx, status);
        break;

    case ProcessState::Error:
        setErrorResponse(conn->_response, 500, "Process Error", ctx.cgiRouteConfig);
        conn->setState(Connection::SendResponse);
        break;

    case ProcessState::Running:
        if (!_readPipeData(ctx, false)) {
            setErrorResponse(conn->_response, 500, "Pipe Error", ctx.cgiRouteConfig);
            conn->setState(Connection::SendResponse);
        }
        break;
    }
}
