#include "ConfigParser.h"
#include "ConfigStructure.h"
#include "HttpRequest.h"
#include "test_stubs.h"
#include "utils.h"
#include "gtest/gtest.h"
#include <Router.h>

Router newRouterTest();

struct RouterTestParams {
    std::string socketAddr;
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

    // // here start to build router ==============================-
    std::map< std::string, IRouter* > routers;
    std::vector< ServerConfig > svrCfgs = cfgPrsr->getServersConfig();
    for (size_t i = 0; i < svrCfgs.size(); i++) {
        ServerConfig svrCfg = svrCfgs[i];
        for (std::set< std::pair<std::string, int> >::iterator it = svrCfg.listen.begin(); it != svrCfg.listen.end(); ++it) {
            std::string ip = it->first;
            int port = it->second;
            std::string key = ip + ":" + to_string(port);

            if (routers.find(key) != routers.end()) {
                addSvrToRouter(routers[key], svrCfg);
            } else {
                std::map< std::string, IHandler* > hdlrs = {
                    {"GET", new StubHandler("GET")},
                    {"POST", new StubHandler("POST")},
                    {"DELETE", new StubHandler("DELETE")},
                };
                IRouter* r = new Router(hdlrs);
                addSvrToRouter(r, svrCfg);
                routers[key] = r;
            }
        }
    }

    IRouter* r = routers[params.socketAddr];
    // here end to build router ==============================-

    Route gotRoute = r->match(request);
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
    for (std::map< std::string, IRouter* >::iterator it = routers.begin(); it != routers.end(); it++) {
        delete it->second;
    }
}

INSTANTIATE_TEST_SUITE_P(
    pathTests, RouterTest,
    ::testing::Values(

        RouterTestParams{"0.0.0.0:8080",
                         HttpRequest{"GET", "/css/themes/", "HTTP/1.1", {{"host", "www.example.com"}}},
                         {"GET", "POST"},
                         {"/dataSecond/static", {}, {}, oneMB, false, {}}},
        RouterTestParams{"0.0.0.0:8080",
                         HttpRequest{"GET", "/css/themes/", "HTTP/1.1", {{"host", "example.com"}}},
                         {"GET", "POST"},
                         {"/data/static", {}, {}, oneMB, false, {}}},
        RouterTestParams{"0.0.0.0:8080",
                         HttpRequest{"GET", "asdf", "HTTP/1.1", {{"host", "www.example.com"}}},
                         {"GET"},
                         {"/var/worldwideweb/html", {}, {}, oneMB, false, {}}},
        RouterTestParams{"0.0.0.0:8080",
                         HttpRequest{"GET", "asdf", "HTTP/1.1", {{"host", "example.com"}}},
                         {"GET"},
                         {"/var/www/html", {}, {}, oneMB, false, {}}},
        RouterTestParams{"0.0.0.0:8081",
                         HttpRequest{"GET", "/css/styles/", "HTTP/1.1", {{"host", "www.test.com"}}},
                         {"GET", "POST", "DELETE"},
                         {"/data/static", {}, {}, oneMB, false, {}}},
        RouterTestParams{"0.0.0.0:8081",
                         HttpRequest{"GET", "/css/", "HTTP/1.1", {{"host", "www.test.com"}}},
                         {"GET", "POST", "DELETE"},
                         {"/data/static", {}, {}, oneMB, false, {}}},

        RouterTestParams{"0.0.0.0:8080",
                         HttpRequest{"GET", "/css/scripts/script.js", "HTTP/1.1", {{"host", "example.com"}}},
                         {"GET", "POST", "DELETE"},
                         {"/data/scripts", {}, {}, 12 * oneMB, false, {}}},
        RouterTestParams{"0.0.0.0:8080",
                         HttpRequest{"GET", "/images/themes/", "HTTP/1.1", {{"host", "example.com"}}},
                         {"GET", "POST", "DELETE"},
                         {"/data", {}, {}, oneMB, false, {{".php", "/usr/bin/php-cgi"}}}},
        RouterTestParams{"0.0.0.0:8080",
                         HttpRequest{"GET", "/css/styles/", "HTTP/1.1", {{"host", "example.com"}}},
                         {"GET"},
                         {"/data/extra", {}, {}, oneKB, false, {}}},
        RouterTestParams{"0.0.0.0:8080",
                         HttpRequest{"GET", "/css", "HTTP/1.1", {{"host", "example.com"}}},
                         {"GET"},
                         {"/var/www/html", {}, {}, oneMB, false, {}}},
        RouterTestParams{"0.0.0.0:8083",
                         HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "test3.com"}}},
                         {"DELETE"},
                         {"/test3/www/html", {}, {}, oneMB, true, {}}},
        RouterTestParams{"0.0.0.0:8080",
                         HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "unknown.com"}}},
                         {"GET"},
                         {"/var/www/html", {}, {}, oneMB, false, {}}},
        RouterTestParams{"0.0.0.0:8081",
                         HttpRequest{"GET", "/images/", "HTTP/1.1", {{"host", "test.com"}}},
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
        RouterTestParams{"0.0.0.0:8081",
                         HttpRequest{"GET", "/js/", "HTTP/1.1", {{"host", "test.com"}}},
                         {"GET"},
                         {"/data/scripts", {}, {}, oneMB, false, {}}},
        RouterTestParams{"0.0.0.0:8081",
                         HttpRequest{"GET", "/keys/", "HTTP/1.1", {{"host", "test.com"}}},
                         {"GET", "POST", "DELETE"},
                         {"/var/www/secure", {"index.html", "index.htm"}, {}, oneMB, false, {}}},
        RouterTestParams{"0.0.0.0:8080",
                         HttpRequest{"GET", "/images/", "HTTP/1.1", {{"host", "example.com"}}},
                         {"GET", "POST", "DELETE"},
                         {"/data", {}, {}, oneMB, false, {}}},
        RouterTestParams{"0.0.0.0:8080",
                         HttpRequest{"GET", "/css/", "HTTP/1.1", {{"host", "example.com"}}},
                         {"GET", "POST"},
                         {"/data/static", {}, {}, oneMB, false, {}}},
        RouterTestParams{"0.0.0.0:8080",
                         HttpRequest{"GET", "/index.html", "HTTP/1.1", {{"host", "example.com"}}},
                         {"GET"},
                         {"/var/www/html", {}, {}, oneMB, false, {}}},
        RouterTestParams{"0.0.0.0:8080",
                         HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "example.com"}}},
                         {"GET"},
                         {"/var/www/html", {}, {}, oneMB, false, {}}},
        RouterTestParams{"0.0.0.0:8081",
                         HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "test.com"}}},
                         {"GET", "POST", "DELETE"},
                         {"/var/www/secure", {"index.html", "index.htm"}, {}, oneMB, false, {}}},
        RouterTestParams{"0.0.0.0:8082",
                         HttpRequest{"GET", "/", "HTTP/1.1", {{"host", "test2.com"}}},
                         {"GET"},
                         {"/usr/share/nginx/html", {}, {}, oneMB, false, {}}}));
