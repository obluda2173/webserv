#ifndef BADREQUESTHANDLER_H
#define BADREQUESTHANDLER_H

#include "Connection.h"
#include "HttpResponse.h"
#include "Router.h"

class BadRequestHandler : public IHandler {
    virtual void handle(Connection* conn, const HttpRequest& req, const RouteConfig& config) {
        (void)req;
        (void)config;
        HttpResponse resp;
        resp.version = "HTTP/1.1";
        resp.statusCode = 400;
        resp.statusMessage = "Bad Request";
        conn->_response = resp;
        conn->setState(Connection::SendResponse);
        return;
    };
};

#endif // BADREQUESTHANDLER_H
