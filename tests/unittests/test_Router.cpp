#include "ConfigParser.h"
#include "HttpRequest.h"
#include "test_stubs.h"
#include "gtest/gtest.h"
#include <Router.h>
#include <unordered_map>

Router newRouterTest();

struct RouterTestParams {
    HttpRequest req;
    std::set< std::string > wantHdlrs;
    RouteConfig wantRouteCfg;
};

class RouterTest : public ::testing::TestWithParam< RouterTestParams > {};

TEST_P(RouterTest, testWithArtificialConfig) {
    RouterTestParams params = GetParam();
    HttpRequest request = params.req;
    RouteConfig wantCfg = params.wantRouteCfg;
    std::set< std::string > wantHdlrs = params.wantHdlrs;

    Router r = newRouterTest();
    Route gotRoute = r.match(request);
    // check root
    EXPECT_EQ(wantCfg.root, gotRoute.cfg.root);
    // check index
    EXPECT_EQ(wantCfg.index.size(), gotRoute.cfg.index.size());
    for (size_t i = 0; i < wantCfg.index.size(); i++)
        EXPECT_EQ(wantCfg.index[i], gotRoute.cfg.index[i]);

    // check clientMaxBody
    EXPECT_EQ(wantCfg.clientMaxBody, gotRoute.cfg.clientMaxBody);
    // check error pages
    EXPECT_EQ(wantCfg.errorPage.size(), gotRoute.cfg.errorPage.size());
    for (auto it = wantCfg.errorPage.begin(); it != wantCfg.errorPage.end(); it++)
        EXPECT_EQ(wantCfg.errorPage[it->first], gotRoute.cfg.errorPage[it->first]);

    // check handlers
    EXPECT_EQ(wantHdlrs.size(), gotRoute.hdlrs.size());
    for (auto it = wantHdlrs.begin(); it != wantHdlrs.end(); ++it)
        EXPECT_EQ(*it, ((StubHandler*)gotRoute.hdlrs.find(*it)->second)->type);
}

TEST_P(RouterTest, testWithConfigParsing) {
    // Parameters
    RouterTestParams params = GetParam();
    HttpRequest request = params.req;
    RouteConfig wantCfg = params.wantRouteCfg;
    std::set< std::string > wantHdlrs = params.wantHdlrs;

    // Building the router
    IConfigParser* cfgPrsr = new ConfigParser("./tests/unittests/test_configs/config1.conf");

    std::map< std::string, IHandler* > hdlrs = {
        {"GET", new StubHandler("GET")},
        {"POST", new StubHandler("POST")},
        {"DELETE", new StubHandler("DELETE")},
    };
    // std::map<int, IRouter*>
    Router r = newRouter(cfgPrsr->getServersConfig(), hdlrs);

    Route gotRoute = r.match(request);
    // check root
    EXPECT_EQ(wantCfg.root, gotRoute.cfg.root);
    // check index
    EXPECT_EQ(wantCfg.index.size(), gotRoute.cfg.index.size());
    for (size_t i = 0; i < wantCfg.index.size(); i++)
        EXPECT_EQ(wantCfg.index[i], gotRoute.cfg.index[i]);

    // check clientMaxBody
    EXPECT_EQ(wantCfg.clientMaxBody, gotRoute.cfg.clientMaxBody);
    // check error pages
    EXPECT_EQ(wantCfg.errorPage.size(), gotRoute.cfg.errorPage.size());
    for (auto it = wantCfg.errorPage.begin(); it != wantCfg.errorPage.end(); it++)
        EXPECT_EQ(wantCfg.errorPage[it->first], gotRoute.cfg.errorPage[it->first]);

    // check handlers
    EXPECT_EQ(wantHdlrs.size(), gotRoute.hdlrs.size());
    for (auto it = wantHdlrs.begin(); it != wantHdlrs.end(); ++it)
        EXPECT_EQ(*it, ((StubHandler*)gotRoute.hdlrs.find(*it)->second)->type);

    // check cgi
    for (std::map< std::string, std::string >::iterator it = wantCfg.cgi.begin(); it != wantCfg.cgi.end(); it++) {
        std::string wantKey = it->first;
        std::string wantVal = it->second;
        std::map< std::string, std::string > gotCgi = gotRoute.cfg.cgi;
        EXPECT_TRUE(gotCgi.find(wantKey) != gotCgi.end());
        EXPECT_EQ(gotCgi[wantKey], wantVal);
    }

    delete cfgPrsr;
}

INSTANTIATE_TEST_SUITE_P(
    pathTests, RouterTest,
    ::testing::Values(RouterTestParams{HttpRequest{"GET", "asdf", "HTTP/1.1", {{"host", "example.com"}}},
                                       {"GET"},
                                       {"/var/www/html", {}, {}, oneMB, false, {}}},
                      RouterTestParams{HttpRequest{"GET", "/css/styles/", "HTTP/1.1", {{"host", "www.test.com"}}},
                                       {"GET", "POST", "DELETE"},
                                       {"/data/static", {}, {}, oneMB, false, {}}},
                      RouterTestParams{HttpRequest{"GET", "/css/", "HTTP/1.1", {{"host", "www.test.com"}}},
                                       {"GET", "POST", "DELETE"},
                                       {"/data/static", {}, {}, oneMB, false, {}}},

                      RouterTestParams{
                          HttpRequest{"GET", "/css/scripts/script.js", "HTTP/1.1", {{"host", "example.com"}}},
                          {"GET", "POST", "DELETE"},
                          {"/data/scripts", {}, {}, 12 * oneMB, false, {}}},
                      RouterTestParams{HttpRequest{"GET", "/images/themes/", "HTTP/1.1", {{"host", "example.com"}}},
                                       {"GET", "POST", "DELETE"},
                                       {"/data", {}, {}, oneMB, false, {{".php", "/usr/bin/php-cgi"}}}},
                      RouterTestParams{HttpRequest{"GET", "/css/themes/", "HTTP/1.1", {{"host", "example.com"}}},
                                       {"GET", "POST"},
                                       {"/data/static", {}, {}, oneMB, false, {}}},
                      RouterTestParams{HttpRequest{"GET", "/css/styles/", "HTTP/1.1", {{"host", "example.com"}}},
                                       {"GET"},
                                       {"/data/extra", {}, {}, oneKB, false, {}}},
                      RouterTestParams{HttpRequest{"GET", "/css", "HTTP/1.1", {{"host", "example.com"}}},
                                       {"GET"},
                                       {"/var/www/html", {}, {}, oneMB, false, {}}},
                      RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "test3.com"}}},
                                       {"DELETE"},
                                       {"/test3/www/html", {}, {}, oneMB, true, {}}},
                      RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "unknown.com"}}},
                                       {"GET"},
                                       {"/var/www/html", {}, {}, oneMB, false, {}}},
                      RouterTestParams{HttpRequest{"GET", "/images/", "HTTP/1.1", {{"host", "test.com"}}},
                                       {"GET", "POST", "DELETE"},
                                       {"/data2",
                                        {},
                                        {{404, "/custom_404.html"},
                                         {500, "/custom_50x.html"},
                                         {502, "/custom_50x.html"},
                                         {503, "/custom_50x.html"},
                                         {504, "/custom_50x.html"}},
                                        oneMB,
                                        false,
                                        {}}},
                      RouterTestParams{HttpRequest{"GET", "/js/", "HTTP/1.1", {{"host", "test.com"}}},
                                       {"GET"},
                                       {"/data/scripts", {}, {}, oneMB, false, {}}},
                      RouterTestParams{HttpRequest{"GET", "/keys/", "HTTP/1.1", {{"host", "test.com"}}},
                                       {"GET", "POST", "DELETE"},
                                       {"/var/www/secure", {"index.html", "index.htm"}, {}, oneMB, false, {}}},
                      RouterTestParams{HttpRequest{"GET", "/images/", "HTTP/1.1", {{"host", "example.com"}}},
                                       {"GET", "POST", "DELETE"},
                                       {"/data", {}, {}, oneMB, false, {}}},
                      RouterTestParams{HttpRequest{"GET", "/css/", "HTTP/1.1", {{"host", "example.com"}}},
                                       {"GET", "POST"},
                                       {"/data/static", {}, {}, oneMB, false, {}}},
                      RouterTestParams{HttpRequest{"GET", "/index.html", "HTTP/1.1", {{"host", "example.com"}}},
                                       {"GET"},
                                       {"/var/www/html", {}, {}, oneMB, false, {}}},
                      RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "example.com"}}},
                                       {"GET"},
                                       {"/var/www/html", {}, {}, oneMB, false, {}}},
                      RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "test.com"}}},
                                       {"GET", "POST", "DELETE"},
                                       {"/var/www/secure", {"index.html", "index.htm"}, {}, oneMB, false, {}}},
                      RouterTestParams{HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "test2.com"}}},
                                       {"GET"},
                                       {"/usr/share/nginx/html", {}, {}, oneMB, false, {}}}));
