#include "CgiHandler.h"
#include "Connection.h"
#include "HttpRequest.h"
#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include <map>

struct CgiTestParams {
    std::vector< std::pair< std::string, std::vector< std::string > > > queryParams;
    std::string wantOutput;
};

class CgiHandlerTestP : public testing::TestWithParam< CgiTestParams > {};

std::string buildUri(std::string script,
                     std::vector< std::pair< std::string, std::vector< std::string > > > queryParams) {
    std::string queryString = "/" + script + "?";
    size_t count = 0;
    for (std::vector< std::pair< std::string, std::vector< std::string > > >::iterator it = queryParams.begin();
         it != queryParams.end(); it++) {
        std::string key = it->first;
        for (size_t i = 0; i < it->second.size(); i++) {
            queryString += it->first + "=" + it->second[i];
            if (i + 1 < it->second.size())
                queryString += "&";
        }

        if ((count + 1) != queryParams.size())
            queryString += "&";
        count++;
    }
    return queryString;
}

std::string getOutput(Connection* conn) {
    std::string gotOutput = "";
    std::vector< char > buffer(1024);
    size_t r = 0;
    while (!conn->_response.body->isDone()) {
        r = conn->_response.body->read(buffer.data(), 1024);
        gotOutput += std::string(buffer.data(), r);
    }
    return gotOutput;
}

TEST_P(CgiHandlerTestP, WithQueryParams) {
    CgiTestParams params = GetParam();
    HttpRequest req = HttpRequest();
    RouteConfig cfg = {"tests/unittests/test_cgi/scripts",       {}, {}, 10000, false,
                       {{"py", "/home/kay/.pyenv/shims/python"}}};
    IHandler* cgiHdlr = new CgiHandler();

    req.uri = buildUri("MqueryParams.py", params.queryParams);

    Connection* conn = new Connection({}, -1, NULL, NULL);
    cgiHdlr->handle(conn, req, cfg);

    std::string gotOutput = getOutput(conn);
    ASSERT_EQ(params.wantOutput, gotOutput);

    delete cgiHdlr;
    delete conn;
}

INSTANTIATE_TEST_SUITE_P(firstTest, CgiHandlerTestP,
                         testing::Values(CgiTestParams{{{"name", {"kay"}}, {"hobby", {"coding", "running"}}},
                                                       {"name kay\nhobby coding, running\n"}},
                                         CgiTestParams{{{"name", {"kay"}}, {"hobby", {"coding"}}},
                                                       {"name kay\nhobby coding\n"}},
                                         CgiTestParams{{{"name", {"kay"}}}, {"name kay\n"}}));

// TEST(CgiHandlerTest, helloWorld) {

//     std::string wantScriptOutput = "Hello, World!";

//     RouteConfig cfg = {"tests/unittests/test_cgi/scripts", {}, {}, 10000, false, {{"php", "/usr/bin/php"}}};
//     IHandler* cgiHdlr = new CgiHandler();

//     HttpRequest req = HttpRequest();
//     req.uri = buildUri("helloWorld.php", {});
//     Connection* conn = new Connection({}, -1, NULL, NULL);
//     cgiHdlr->handle(conn, req, cfg);

//     std::string gotOutput = getOutput(conn);
//     ASSERT_EQ(wantScriptOutput, gotOutput);

//     delete cgiHdlr;
//     delete conn;
// }
