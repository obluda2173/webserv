#include "HttpRequest.h"
#include "gtest/gtest.h"
#include <Router.h>

TEST(RouterTests, firstTests) {
    std::map<std::string, std::vector<std::string>> svrNameToLocPrefixes;
    svrNameToLocPrefixes["example.com"] = std::vector<std::string>{};
    Router router(svrNameToLocPrefixes);

    HttpRequest request;
    request.method = "GET";
    request.version = "1.1";
    request.headers["host"] = "example.com";
    router.match(request);
}
