#ifndef PINGHANDLER_H
#define PINGHANDLER_H

#include "HttpResponse.h"
#include "Router.h"
#include "Connection.h"

class PingHandler : public IHandler {
    virtual void handle(Connection* conn, const HttpRequest& req, const RouteConfig& config) {
        (void)req;
        (void)config;
        HttpResponse resp;
        resp.statusCode = 200;
        resp.statusMessage = "OK";
        resp.contentLength = 4;
        resp.body = "pong";
        resp.version = "HTTP/1.1";
        conn->_response = resp;
        conn->setStateToSendResponse();
        return;
    };
};

#endif // PINGHANDLER_H
