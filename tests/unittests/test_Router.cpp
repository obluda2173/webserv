#include "ConfigParser.h"
#include "HttpRequest.h"
#include "gtest/gtest.h"
#include <Router.h>

struct RouterTestParams {
    HttpRequest req;
    std::string wantPath;
};

class RouterTest : public ::testing::TestWithParam<RouterTestParams> {};

TEST_P(RouterTest, pathTests) {
    RouterTestParams params = GetParam();
    HttpRequest request = params.req;
    std::string wantPath = params.wantPath;

    // rather use the ConfigParser
    IConfigParser* cfgPrsr = new ConfigParser("./tests/unittests/test_configs/config1.conf");
    cfgPrsr->getServersConfig();

    Router router = newRouter(cfgPrsr->getServersConfig());

    GetHandler getHdlr = router.match(request);
    EXPECT_EQ(wantPath, getHdlr.getPath());
    delete cfgPrsr;
}

INSTANTIATE_TEST_SUITE_P(
    pathTests, RouterTest,
    ::testing::Values(
        RouterTestParams{HttpRequest{"GET", "/css/styles/", "HTTP/1.1", {{"host", "www.test.com"}}},
                         "/data/static/css/styles/"},
        RouterTestParams{HttpRequest{"GET", "/css/styles/", "HTTP/1.1", {{"host", "test.com"}}},
                         "/data/static/css/styles/"},
        RouterTestParams{HttpRequest{"GET", "/images/", "HTTP/1.1", {{"host", "test.com"}}}, "/data2/images/"},
        RouterTestParams{HttpRequest{"GET", "/css/", "HTTP/1.1", {{"host", "test.com"}}}, "/data/static/css/"},
        RouterTestParams{HttpRequest{"GET", "/css/scripts/script.js", "HTTP/1.1", {{"host", "example.com"}}},
                         "/data/scripts/css/scripts/script.js"},
        RouterTestParams{HttpRequest{"GET", "/css/styles/", "HTTP/1.1", {{"host", "example.com"}}},
                         "/data/extra/css/styles/"},
        RouterTestParams{HttpRequest{"GET", "/images/screenshots/", "HTTP/1.1", {{"host", "example.com"}}},
                         "/data/images/screenshots/"},
        RouterTestParams{HttpRequest{"GET", "/images/themes/", "HTTP/1.1", {{"host", "example.com"}}},
                         "/data/images/themes/"},
        RouterTestParams{HttpRequest{"GET", "/css/themes/", "HTTP/1.1", {{"host", "example.com"}}},
                         "/data/static/css/themes/"},
        RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "unknown.com"}}}, "/var/www/html/"},
        RouterTestParams{HttpRequest{"GET", "/css", "HTTP/1.1", {{"host", "example.com"}}}, "/var/www/html/css"},
        RouterTestParams{HttpRequest{"GET", "/js/", "HTTP/1.1", {{"host", "test.com"}}}, "/data/scripts/js/"},
        RouterTestParams{HttpRequest{"GET", "/keys/", "HTTP/1.1", {{"host", "test.com"}}}, "/var/www/secure/keys/"},
        RouterTestParams{HttpRequest{"GET", "/images/", "HTTP/1.1", {{"host", "example.com"}}}, "/data/images/"},
        RouterTestParams{HttpRequest{"GET", "/css/", "HTTP/1.1", {{"host", "example.com"}}}, "/data/static/css/"},
        RouterTestParams{HttpRequest{"GET", "/index.html", "HTTP/1.1", {{"host", "example.com"}}},
                         "/var/www/html/index.html"},
        RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "example.com"}}}, "/var/www/html/"},
        RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "test.com"}}}, "/var/www/secure/"},
        RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "test2.com"}}}, "/usr/share/nginx/html/"}));
