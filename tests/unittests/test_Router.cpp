#include "ConfigStructure.h"
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

    Router router(std::map<std::string, std::string>{{"example.com/", "/var/www/html"},
                                                     {"example.com/images/", "/data"},
                                                     {"example.com/css/", "/data/static"},
                                                     {"test.com/", "/var/www/secure"},
                                                     {"test.com/js/", "/data/scripts"},
                                                     {"test2.com/", "/usr/share/nginx/html"}});

    GetHandler getHdlr = router.match(request);
    EXPECT_EQ(wantPath, getHdlr.getPath());
}

INSTANTIATE_TEST_SUITE_P(
    pathTests, RouterTest,
    ::testing::Values(
        // RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "unknown.com"}}},
        // "/var/www/html/"}, the next Request does not fully match location /css/, there fore
        // /var/www/html/css RouterTestParams{HttpRequest{"GET", "/css", "HTTP/1.1", {{"host",
        // "example.com"}}}, "/var/www/html/css"}, the next Request fully matches location /css/
        // RouterTestParams{HttpRequest{"GET", "/js/", "HTTP/1.1", {{"host", "test.com"}}}, "/data/scripts/js/"},
        // RouterTestParams{HttpRequest{"GET", "/images/", "HTTP/1.1", {{"host", "test.com"}}},
        // "/var/www/secure/images/"}, RouterTestParams{HttpRequest{"GET", "/images/", "HTTP/1.1", {{"host",
        // "example.com"}}}, "/data/images/"},
        // RouterTestParams{HttpRequest{"GET", "/css/", "HTTP/1.1", {{"host", "example.com"}}}, "/data/static/css/"},
        RouterTestParams{HttpRequest{"GET", "/index.html", "HTTP/1.1", {{"host", "example.com"}}},
                         "/var/www/html/index.html"},
        RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "example.com"}}}, "/var/www/html/"},
        RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "test.com"}}}, "/var/www/secure/"},
        RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "test2.com"}}}, "/usr/share/nginx/html/"}));
