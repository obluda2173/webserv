#include "ConfigParser.h"
#include "HttpRequest.h"
#include "test_stubs.h"
#include "gtest/gtest.h"
#include <Router.h>
#include <unordered_map>

Router newRouterTest();

struct RouterTestParams {
    HttpRequest req;
    Route wantRoute;
};

class RouterTest : public ::testing::TestWithParam<RouterTestParams> {};

Route newRouteTesting(std::set<std::string> hdlrTypes, std::string root) {
    std::unordered_map<std::string, IHandler&> hdlrs;
    for (std::set<std::string>::iterator itHdlrType = hdlrTypes.begin(); itHdlrType != hdlrTypes.end(); itHdlrType++) {
        StubHandler hdlr(*itHdlrType);
        hdlrs.emplace(*itHdlrType, hdlr);
    }
    return Route{hdlrs, RouteConfig{root}};
}

TEST_P(RouterTest, pathTests) {
    RouterTestParams params = GetParam();
    HttpRequest request = params.req;
    Route wantRoute = params.wantRoute;

    Router r = newRouterTest();
    Route gotRoute = r.match(request);
    EXPECT_EQ(wantRoute.cfg.root, gotRoute.cfg.root);
    EXPECT_EQ(wantRoute.hdlrs.size(), gotRoute.hdlrs.size());
}

TEST_P(RouterTest, testWithConfigParsing) {
    RouterTestParams params = GetParam();
    HttpRequest request = params.req;
    Route wantRoute = params.wantRoute;

    IConfigParser* cfgPrsr = new ConfigParser("./tests/unittests/test_configs/config1.conf");
    cfgPrsr->getServersConfig();
    Router r = newRouter(cfgPrsr->getServersConfig(), new StubHandler("GET"), new StubHandler("POST"), new StubHandler("DELETE"));
    Route gotRoute = r.match(request);
    EXPECT_EQ(wantRoute.cfg.root, gotRoute.cfg.root);
    delete cfgPrsr;
}

INSTANTIATE_TEST_SUITE_P(
    pathTests, RouterTest,
    ::testing::Values(
        // RouterTestParams{HttpRequest{"GET", "/css/scripts/script.js", "HTTP/1.1", {{"host", "unknown.com"}}},
        //                  "/data/scripts/css/scripts/script.js", "GET"},
        // RouterTestParams{HttpRequest{"POST", "/css/scripts/script.js", "HTTP/1.1", {{"host", "example.com"}}},
        //                  "/data/scripts/css/scripts/script.js", "POST"},
        // RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "test3.com"}}}, "/test3/www/html/", "GET"},
        // RouterTestParams{HttpRequest{"POST", "/index.html", "HTTP/1.1", {{"host", "example.com"}}}, "",
        //                  "ERROR"}, // Fuzzy test but it pointed me to an issue with servers that don't have a root
        // RouterTestParams{HttpRequest{"POST", "/js/something", "HTTP/1.1", {{"host", "test.com"}}}, "", "ERROR"},
        // RouterTestParams{HttpRequest{"POST", "/js/", "HTTP/1.1", {{"host", "test.com"}}}, "", "ERROR"},
        // RouterTestParams{HttpRequest{"POST", "/", "HTTP/1.1", {{"host", "test.com"}}}, "/var/www/secure/", "POST"},
        // RouterTestParams{HttpRequest{"POST", "/", "HTTP/1.1", {{"host", "test2.com"}}}, "", "ERROR"},
        // RouterTestParams{HttpRequest{"GET", "/css/styles/", "HTTP/1.1", {{"host", "test.com"}}},
        //                  "/data/static/css/styles/", "GET"},
        // RouterTestParams{HttpRequest{"GET", "/images/", "HTTP/1.1", {{"host", "test.com"}}}, "/data2/images/",
        // "GET"}, RouterTestParams{HttpRequest{"GET", "/css/", "HTTP/1.1", {{"host", "test.com"}}},
        // "/data/static/css/", "GET"}, RouterTestParams{HttpRequest{"GET", "/css/scripts/script.js", "HTTP/1.1",
        // {{"host", "example.com"}}},
        //                  "/data/scripts/css/scripts/script.js", "GET"},
        // RouterTestParams{HttpRequest{"GET", "/css/styles/", "HTTP/1.1", {{"host", "example.com"}}},
        //                  "/data/extra/css/styles/", "GET"},
        // RouterTestParams{HttpRequest{"GET", "/images/screenshots/", "HTTP/1.1", {{"host", "example.com"}}},
        //                  "/data/images/screenshots/", "GET"},
        // RouterTestParams{HttpRequest{"GET", "/images/themes/", "HTTP/1.1", {{"host", "example.com"}}},
        //                  "/data/images/themes/", "GET"},
        // RouterTestParams{HttpRequest{"GET", "/css/themes/", "HTTP/1.1", {{"host", "example.com"}}},
        //                  "/data/static/css/themes/", "GET"},
        // RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "unknown.com"}}}, "/var/www/html/", "GET"},
        // RouterTestParams{HttpRequest{"GET", "/css", "HTTP/1.1", {{"host", "example.com"}}}, "/var/www/html/css",
        // "GET"}, RouterTestParams{HttpRequest{"GET", "/js/", "HTTP/1.1", {{"host", "test.com"}}}, "/data/scripts/js/",
        // "GET"}, RouterTestParams{HttpRequest{"GET", "/keys/", "HTTP/1.1", {{"host", "test.com"}}},
        // "/var/www/secure/keys/",
        //                  "GET"},
        // RouterTestParams{HttpRequest{"GET", "/images/", "HTTP/1.1", {{"host", "example.com"}}}, "/data/images/",
        // "GET"}, RouterTestParams{HttpRequest{"GET", "/css/", "HTTP/1.1", {{"host", "example.com"}}},
        // "/data/static/css/",
        //                  "GET"},
        // RouterTestParams{HttpRequest{"GET", "/index.html", "HTTP/1.1", {{"host", "example.com"}}},
        //                  "/var/www/html/index.html", "GET"},
        // RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "example.com"}}}, "/var/www/html/", "GET"},
        RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "example.com"}}}, newRouteTesting({"GET", "POST", "DELETE"}, "/var/www/html")},
        RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "test.com"}}}, newRouteTesting({"GET", "POST", "DELETE"}, "/var/www/secure")},
        RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "test2.com"}}}, newRouteTesting({"GET"}, "/usr/share/nginx/html")}));
