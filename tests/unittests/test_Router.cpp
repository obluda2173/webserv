#include "ConfigParser.h"
#include "HttpRequest.h"
#include "gtest/gtest.h"
#include <Router.h>

struct RouterTestParams {
    HttpRequest req;
    std::string wantPath;
    std::string wantExecType;
};

class RouterTest : public ::testing::TestWithParam<RouterTestParams> {};

TEST_P(RouterTest, pathTests) {
    RouterTestParams params = GetParam();
    HttpRequest request = params.req;
    std::string wantPath = params.wantPath;
    std::string wantExecType = params.wantExecType;

    // rather use the ConfigParser
    IConfigParser* cfgPrsr = new ConfigParser("./tests/unittests/test_configs/config1.conf");
    cfgPrsr->getServersConfig();

    Router router = newRouter(cfgPrsr->getServersConfig());
    // Router router = newRouterTest();
    ExecutionInfo execInfo = router.match(request);
    EXPECT_EQ(wantPath, execInfo.getDirPath());
    EXPECT_EQ(wantExecType, execInfo.getExecType());
    delete cfgPrsr;
}

INSTANTIATE_TEST_SUITE_P(
    pathTests, RouterTest,
    ::testing::Values(
        RouterTestParams{HttpRequest{"GET", "/css/scripts/script.js", "HTTP/1.1", {{"host", "unknown.com"}}},
                         "/data/scripts/css/scripts/script.js", "GET"},
        RouterTestParams{HttpRequest{"POST", "/css/scripts/script.js", "HTTP/1.1", {{"host", "example.com"}}},
                         "/data/scripts/css/scripts/script.js", "POST"},
        RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "test3.com"}}}, "/test3/www/html/", "GET"},
        RouterTestParams{HttpRequest{"POST", "/index.html", "HTTP/1.1", {{"host", "example.com"}}}, "",
                         "ERROR"}, // Fuzzy test but it pointed me to an issue with servers that don't have a root
        RouterTestParams{HttpRequest{"POST", "/js/something", "HTTP/1.1", {{"host", "test.com"}}}, "", "ERROR"},
        RouterTestParams{HttpRequest{"POST", "/js/", "HTTP/1.1", {{"host", "test.com"}}}, "", "ERROR"},
        RouterTestParams{HttpRequest{"POST", "/", "HTTP/1.1", {{"host", "test.com"}}}, "/var/www/secure/", "POST"},
        RouterTestParams{HttpRequest{"POST", "/", "HTTP/1.1", {{"host", "test2.com"}}}, "", "ERROR"},
        RouterTestParams{HttpRequest{"GET", "/css/styles/", "HTTP/1.1", {{"host", "test.com"}}},
                         "/data/static/css/styles/", "GET"},
        RouterTestParams{HttpRequest{"GET", "/images/", "HTTP/1.1", {{"host", "test.com"}}}, "/data2/images/", "GET"},
        RouterTestParams{HttpRequest{"GET", "/css/", "HTTP/1.1", {{"host", "test.com"}}}, "/data/static/css/", "GET"},
        RouterTestParams{HttpRequest{"GET", "/css/scripts/script.js", "HTTP/1.1", {{"host", "example.com"}}},
                         "/data/scripts/css/scripts/script.js", "GET"},
        RouterTestParams{HttpRequest{"GET", "/css/styles/", "HTTP/1.1", {{"host", "example.com"}}},
                         "/data/extra/css/styles/", "GET"},
        RouterTestParams{HttpRequest{"GET", "/images/screenshots/", "HTTP/1.1", {{"host", "example.com"}}},
                         "/data/images/screenshots/", "GET"},
        RouterTestParams{HttpRequest{"GET", "/images/themes/", "HTTP/1.1", {{"host", "example.com"}}},
                         "/data/images/themes/", "GET"},
        RouterTestParams{HttpRequest{"GET", "/css/themes/", "HTTP/1.1", {{"host", "example.com"}}},
                         "/data/static/css/themes/", "GET"},
        RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "unknown.com"}}}, "/var/www/html/", "GET"},
        RouterTestParams{HttpRequest{"GET", "/css", "HTTP/1.1", {{"host", "example.com"}}}, "/var/www/html/css", "GET"},
        RouterTestParams{HttpRequest{"GET", "/js/", "HTTP/1.1", {{"host", "test.com"}}}, "/data/scripts/js/", "GET"},
        RouterTestParams{HttpRequest{"GET", "/keys/", "HTTP/1.1", {{"host", "test.com"}}}, "/var/www/secure/keys/",
                         "GET"},
        RouterTestParams{HttpRequest{"GET", "/images/", "HTTP/1.1", {{"host", "example.com"}}}, "/data/images/", "GET"},
        RouterTestParams{HttpRequest{"GET", "/css/", "HTTP/1.1", {{"host", "example.com"}}}, "/data/static/css/",
                         "GET"},
        RouterTestParams{HttpRequest{"GET", "/index.html", "HTTP/1.1", {{"host", "example.com"}}},
                         "/var/www/html/index.html", "GET"},
        RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "example.com"}}}, "/var/www/html/", "GET"},
        RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "test.com"}}}, "/var/www/secure/", "GET"},
        RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "test2.com"}}}, "/usr/share/nginx/html/",
                         "GET"}));
