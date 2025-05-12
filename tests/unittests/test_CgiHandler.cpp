#include "CgiHandler.h"
#include "Connection.h"
#include "HttpRequest.h"
#include "gtest/gtest.h"
#include <gtest/gtest.h>

struct CgiTestParams {
    std::vector< std::pair< std::string, std::string > > queryParams;
    std::string wantOutput;
};

class CgiHandlerTestP : public testing::TestWithParam< CgiTestParams > {};

TEST_P(CgiHandlerTestP, first) {
    CgiTestParams params = GetParam();
    HttpRequest req = HttpRequest();
    RouteConfig cfg = {"tests/unittests/test_cgi/scripts",       {}, {}, 10000, false,
                       {{"py", "/home/kay/.pyenv/shims/python"}}};
    IHandler* cgiHdlr = new CgiHandler();

    req.uri = "/MqueryParams.py?";
    for (std::vector< std::pair< std::string, std::string > >::iterator it = params.queryParams.begin();
         it != params.queryParams.end(); it++) {
        req.uri += it->first + "=" + it->second;
        if (it + 1 != params.queryParams.end())
            req.uri += "&";
    }

    // name=John&age=30&hobby=coding&hobby=reading";
    Connection* conn = new Connection({}, -1, NULL, NULL);
    cgiHdlr->handle(conn, req, cfg);

    std::string gotOutput = "";
    std::vector< char > buffer(1024);
    size_t r = 0;
    while (!conn->_response.body->isDone()) {
        r = conn->_response.body->read(buffer.data(), 1024);
        gotOutput += std::string(buffer.data(), r);
    }

    ASSERT_EQ(params.wantOutput, gotOutput);

    delete cgiHdlr;
    delete conn;
}

INSTANTIATE_TEST_SUITE_P(firstTest, CgiHandlerTestP,
                         testing::Values(CgiTestParams{{{"name", "kay"}, {"hobby", "coding"}},
                                                       {"name kay\nhobby coding\n"}},
                                         CgiTestParams{{{"name", "kay"}}, {"name kay\n"}}));

// TEST(CgiHandlerTest, queryParams) {

//     std::string wantScriptOutput = "helloWorld";

//     RouteConfig cfg = {"tests/unittests/test_cgi/scripts",       {}, {}, 10000, false,
//                        {{"py", "/home/kay/.pyenv/shims/python"}}};
//     IHandler* cgiHdlr = new CgiHandler();

//     HttpRequest req = HttpRequest();
//     req.uri = "/1queryParam.py?q=helloWorld";
//     Connection* conn = new Connection({}, -1, NULL, NULL);
//     cgiHdlr->handle(conn, req, cfg);

//     std::string gotOutput = "";
//     std::vector< char > buffer(1024);
//     size_t r = 0;
//     while (!conn->_response.body->isDone()) {
//         r = conn->_response.body->read(buffer.data(), 1024);
//         gotOutput += std::string(buffer.data(), r);
//     }

//     ASSERT_EQ(wantScriptOutput, gotOutput);

//     delete cgiHdlr;
//     delete conn;
// }

// TEST(CgiHandlerTest, mulitpleQueryParams) {

//     std::string wantScriptOutput = "name John\n"
//                                    "age 30\n"
//                                    "hobby coding, reading\n";

//     RouteConfig cfg = {"tests/unittests/test_cgi/scripts",       {}, {}, 10000, false,
//                        {{"py", "/home/kay/.pyenv/shims/python"}}};
//     IHandler* cgiHdlr = new CgiHandler();

//     HttpRequest req = HttpRequest();
//     req.uri = "/MqueryParams.py?name=John&age=30&hobby=coding&hobby=reading";
//     Connection* conn = new Connection({}, -1, NULL, NULL);
//     cgiHdlr->handle(conn, req, cfg);

//     std::string gotOutput = "";
//     std::vector< char > buffer(1024);
//     size_t r = 0;
//     while (!conn->_response.body->isDone()) {
//         r = conn->_response.body->read(buffer.data(), 1024);
//         gotOutput += std::string(buffer.data(), r);
//     }

//     ASSERT_EQ(wantScriptOutput, gotOutput);

//     delete cgiHdlr;
//     delete conn;
// }

// TEST(CgiHandlerTest, helloWorld) {

//     std::string wantScriptOutput = "Hello, World!";

//     RouteConfig cfg = {"tests/unittests/test_cgi/scripts", {}, {}, 10000, false, {{"php", "/usr/bin/php"}}};
//     IHandler* cgiHdlr = new CgiHandler();

//     HttpRequest req = HttpRequest();
//     req.uri = "/helloWorld.php";
//     Connection* conn = new Connection({}, -1, NULL, NULL);
//     cgiHdlr->handle(conn, req, cfg);

//     std::string gotOutput = "";
//     std::vector< char > buffer(1024);
//     size_t r = 0;
//     while (!conn->_response.body->isDone()) {
//         r = conn->_response.body->read(buffer.data(), 1024);
//         gotOutput += std::string(buffer.data(), r);
//     }

//     ASSERT_EQ(wantScriptOutput, gotOutput);

//     delete cgiHdlr;
//     delete conn;
// }
