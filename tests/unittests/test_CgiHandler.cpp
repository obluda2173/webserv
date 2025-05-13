#include "CgiHandler.h"
#include "Connection.h"
#include "HttpRequest.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <gtest/gtest.h>

struct CgiTestParams {
    std::string scriptName;
    std::vector< std::pair< std::string, std::vector< std::string > > > queryParams;
    std::string wantOutput;
};

class CgiHandlerTestP : public testing::TestWithParam< CgiTestParams > {};

TEST_P(CgiHandlerTestP, WithQueryParams) {
    CgiTestParams params = GetParam();
    HttpRequest req = HttpRequest();
    RouteConfig cfg = {"tests/unittests/test_cgi/scripts",
                       {},
                       {},
                       10000,
                       false,
                       {{"php", "/usr/bin/php"}, {"py", "/home/kfreyer/.pyenv/shims/python"}}};
    IHandler* cgiHdlr = new CgiHandler();

    req.uri = buildUri(params.scriptName, params.queryParams);

    Connection* conn = new Connection({}, -1, NULL, NULL);
    cgiHdlr->handle(conn, req, cfg);

    std::string gotOutput = getOutput(conn);
    ASSERT_EQ(params.wantOutput, gotOutput);

    delete cgiHdlr;
    delete conn;
}

INSTANTIATE_TEST_SUITE_P(firstTest, CgiHandlerTestP,
                         testing::Values(CgiTestParams{"MqueryParams.py",
                                                       {{"name", {"kay"}}, {"hobby", {"coding", "running"}}},
                                                       {"name kay\nhobby coding, running\n"}},
                                         CgiTestParams{"MqueryParams.py",
                                                       {{"name", {"kay"}}, {"hobby", {"coding"}}},
                                                       {"name kay\nhobby coding\n"}},
                                         CgiTestParams{"MqueryParams.py", {{"name", {"kay"}}}, {"name kay\n"}},
                                         CgiTestParams{"helloWorld.php", {}, {"Hello, World!"}}));
