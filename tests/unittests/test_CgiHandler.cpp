#include "CgiHandler.h"
#include "Connection.h"
#include "HttpRequest.h"
#include <gtest/gtest.h>

TEST(CgiHandlerTest, firstTest) {

    std::string wantScriptOutput = "Hello, World!";

    RouteConfig cfg = {"tests/unittests/test_cgi/scripts", {}, {}, 10000, false, {{"php", "/usr/bin/php"}}};
    IHandler* cgiHdlr = new CgiHandler();

    HttpRequest req = HttpRequest();
    req.uri = "/helloWorld.php";
    Connection* conn = new Connection({}, -1, NULL, NULL);
    cgiHdlr->handle(conn, req, cfg);

    std::string gotOutput = "";
    std::vector< char > buffer(1024);
    size_t r = 0;
    while (!conn->_response.body->isDone()) {
        r = conn->_response.body->read(buffer.data(), 1024);
        gotOutput += std::string(buffer.data(), r);
    }

    ASSERT_EQ(wantScriptOutput, gotOutput);

    delete cgiHdlr;
    delete conn;
}
