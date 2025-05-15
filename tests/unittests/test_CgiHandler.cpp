#include "CgiHandler.h"
#include "Connection.h"
#include "HttpRequest.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include <thread>

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
                       {{"php", "/usr/bin/php-cgi"}, {"py", "/usr/bin/python3"}}};
    // IHandler* cgiHdlr = new CgiHandler();
    CgiHandler cgiHdlr;

    req.uri = buildUri(params.scriptName, params.queryParams);

    Connection* conn = new Connection({}, -1, NULL, NULL);
    cgiHdlr.handle(conn, req, cfg);

    bool cgiCompleted = false;
    auto startTime = std::chrono::steady_clock::now();
    while (!cgiCompleted) {
        if (conn->getState() == Connection::SendResponse) {
            cgiCompleted = true;
            break;
        }
        if (conn->getState() == Connection::HandlingCgi) {
            cgiHdlr.handleCgiProcess(conn);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        auto now = std::chrono::steady_clock::now();
        if (now - startTime > std::chrono::seconds(5)) {
            FAIL() << "CGI processing timed out";
        }
    }

    std::string gotOutput = getOutput(conn);
    ASSERT_EQ(params.wantOutput, gotOutput);

    delete conn;
}

INSTANTIATE_TEST_SUITE_P(firstTest, CgiHandlerTestP,
                         testing::Values(CgiTestParams{"MqueryParams.py",
                                                       {{"name", {"kay"}}, {"hobby", {"coding", "running"}}},
                                                       {"name kay\nhobby coding, running\n"}},
                                         CgiTestParams{"MqueryParams.py",
                                                       {{"name", {"kay"}}, {"hobby", {"coding"}}},
                                                       {"name kay\nhobby coding\n"}},
                                         CgiTestParams{"MqueryParams.py", {{"name", {"kay"}}}, {"name kay\n"}}
                                         // CgiTestParams{"helloWorld.php", {}, {"Hello, World!"}}
                                         ));
