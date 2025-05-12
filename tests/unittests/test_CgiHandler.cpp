#include "CgiHandler.h"
#include "Connection.h"
#include "HttpRequest.h"
#include "Router.h"
#include <gtest/gtest.h>

TEST(CgiHandler, firstTest) {

    RouteConfig cfg = {"test/unittests/test_cgi/scripts", {}, {}, 10000, false, {"php", "/usr/bin/php"}};
    IHandler* cgiHdlr = new CgiHandler();

    HttpRequest req = HttpRequest();
    Connection* conn = new Connection({}, -1, NULL, NULL);

    cgiHdlr->handle(conn, req, cfg);

    std::string gotOutput = "";
    std::vector< char > buffer(1024);
    size_t r = 0;
    while (!conn->_response.body->isDone()) {
        r = conn->_response.body->read(gotOutput.data(), 1024);
        gotOutput += std::string(gotOutput.data(), r);
    }

    delete cgiHdlr;
    delete conn;
}
