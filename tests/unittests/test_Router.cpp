#include "HttpRequest.h"
#include "gtest/gtest.h"
#include <Router.h>

struct RouterTestParams {
    HttpRequest request;
    std::map<std::string, std::vector<std::string>> svrNameToLocPrefixes;
    std::string wantLoc;
};

class RouterTests : public ::testing::TestWithParam<RouterTestParams> {};

TEST_P(RouterTests, firstTests) {
    RouterTestParams params = GetParam();
    std::map<std::string, std::vector<std::string>> svrNameToLocPrefixes = params.svrNameToLocPrefixes;
    HttpRequest request = params.request;
    std::string wantLoc = params.wantLoc;

    Router router(svrNameToLocPrefixes);

    std::string gotLoc = router.match(request);
    EXPECT_STREQ(wantLoc.c_str(), gotLoc.c_str());
}

INSTANTIATE_TEST_SUITE_P(
    firstTests, RouterTests,
    ::testing::Values(RouterTestParams{HttpRequest{"GET", "", "1.1", {{"host", "example.com"}}},
                                       {{"example.com", std::vector<std::string>{}}},
                                       ""},
                      RouterTestParams{
                          HttpRequest{"GET", "", "1.1", {{"host", "example.com"}}},
                          {{"example.com", std::vector<std::string>{}}, {"test.com", std::vector<std::string>{}}},
                          ""},
                      RouterTestParams{HttpRequest{"GET", "/images/photos/dog.jpg", "1.1", {{"host", "example.com"}}},
                                       {{"example.com", std::vector<std::string>{"/", "/images/", "/images/photos/"}}},
                                       "/images/photos/"},

                      RouterTestParams{HttpRequest{"GET", "/docs/report.pdf", "1.1", {{"host", "example.com"}}},
                                       {{"example.com", std::vector<std::string>{"/images/"}}},
                                       ""}

                      ));
