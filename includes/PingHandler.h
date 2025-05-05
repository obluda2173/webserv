#ifndef PINGHANDLER_H
#define PINGHANDLER_H

#include "Connection.h"
#include "HttpResponse.h"
#include "Router.h"

class PingHandler : public IHandler {
    virtual void handle(Connection* conn, const HttpRequest& req, const RouteConfig& config) {
        (void)req;
        (void)config;
        HttpResponse& resp = conn->_response;
        resp.statusCode = 200;
        resp.statusMessage = "OK";
        resp.contentLength = 4;
        resp.body = new StringBodyProvider("pong");
        resp.version = "HTTP/1.1";
        conn->setState(Connection::SendResponse);
        return;
    };
};

#endif // PINGHANDLER_H
