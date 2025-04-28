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

    Router router(std::map<std::string, std::string>{
        {"example.com", "/var/www/html"},
        {"test.com", "/var/www/images"},
        {"test2.com", "/var/www/photos"},
    });

    GetHandler getHdlr = router.match(request);
    EXPECT_EQ(wantPath, getHdlr.getPath());
}

INSTANTIATE_TEST_SUITE_P(
    pathTests, RouterTest,
    ::testing::Values(
        // RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "unknown.com"}}}, "/var/www/html/"},
        RouterTestParams{HttpRequest{"GET", "/css/", "HTTP/1.1", {{"host", "example.com"}}}, "/data/static/css/"},
        RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "example.com"}}}, "/var/www/html/"},
        RouterTestParams{HttpRequest{"GET", "/images/", "HTTP/1.1", {{"host", "example.com"}}},
                         "/var/www/html/images/"},
        RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "test.com"}}}, "/var/www/images/"},
        RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "test2.com"}}}, "/var/www/photos/"}));

class RouterTest2 : public ::testing::TestWithParam<RouterTestParams> {};

TEST_P(RouterTest2, pathTests) {
    RouterTestParams params = GetParam();
    HttpRequest request = params.req;
    std::string wantPath = params.wantPath;

    Router router(std::map<std::string, std::string>{
        {"test.de", "/var/www/images"},
        {"test2.de", "/var/www/photos"},
        {"example.de", "/var/www/html"},
    });

    GetHandler getHdlr = router.match(request);
    EXPECT_EQ(wantPath, getHdlr.getPath());
}

INSTANTIATE_TEST_SUITE_P(
    pathTests, RouterTest2,
    ::testing::Values(
        RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "example.de"}}}, "/var/www/html/"},
        RouterTestParams{HttpRequest{"GET", "/images/", "HTTP/1.1", {{"host", "example.de"}}}, "/var/www/html/images/"},
        RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "test.de"}}}, "/var/www/images/"},
        RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "test2.de"}}}, "/var/www/photos/"}));
