#include "test_stubs.h"
#include <Router.h>

Router newRouterTest() {
    std::string defaultSvr;
    std::set< std::string > svrs;
    std::map< std::string, std::set< std::string > > svrToLocs;
    std::map< std::string, std::set< std::string > > routeToAllowedMethods;

    defaultSvr = "example.com";

    svrs = {"example.com", "test.com", "www.test.com", "test2.com", "test3.com"};

    svrToLocs = {
        {"example.com", {"/images/", "/css/scripts/", "/css/", "/css/styles/"}},
        {"test.com", {"/css/", "/js/", "/images/"}},
        {"www.test.com", {"/css/", "/js/", "/images/"}},
        {"test2.com", {}},
        {"test3.com", {"/"}},

    };

    std::map< std::string, IHandler* > hdlrs = {
        {"GET", new StubHandler("GET")},
        {"POST", new StubHandler("POST")},
        {"DELETE", new StubHandler("DELETE")},
    };

    std::map< std::string, Route > uriToRoutes;
    uriToRoutes = {{"www.test.com/css/",
                    {{{"GET", hdlrs["GET"]}, {"POST", hdlrs["POST"]}, {"DELETE", hdlrs["DELETE"]}},
                     {"/data/static", {}, {}, oneMB, false, {}}}},

                   {"example.com/css/scripts/",
                    {{{"GET", hdlrs["GET"]}, {"POST", hdlrs["POST"]}, {"DELETE", hdlrs["DELETE"]}},
                     {"/data/scripts", {}, {}, 12 * oneMB, false, {}}}},
                   {"example.com/css/styles/", {{{"GET", hdlrs["GET"]}}, {"/data/extra", {}, {}, oneKB, false, {}}}},
                   {"test3.com/", {{{"DELETE", hdlrs["DELETE"]}}, {"/test3/www/html", {}, {}, oneMB, false, {}}}},
                   {"test.com/images/",
                    {{{"GET", hdlrs["GET"]}, {"POST", hdlrs["POST"]}, {"DELETE", hdlrs["DELETE"]}},
                     {"/data2",
                      {},
                      {{404, "/custom_404.html"},
                       {500, "/custom_50x.html"},
                       {502, "/custom_50x.html"},
                       {503, "/custom_50x.html"},
                       {504, "/custom_50x.html"}},
                      oneMB,
                      false,
                      {}}}},
                   {"test.com/js/", {{{"GET", hdlrs["GET"]}}, {"/data/scripts", {}, {}, oneMB, false, {}}}},
                   {"example.com/images/",
                    {{{"GET", hdlrs["GET"]}, {"POST", hdlrs["POST"]}, {"DELETE", hdlrs["DELETE"]}},
                     {"/data", {}, {}, oneMB, false, {}}}},
                   {"example.com/css/",
                    {{{"GET", hdlrs["GET"]}, {"POST", hdlrs["POST"]}}, {"/data/static", {}, {}, oneMB, false, {}}}},
                   {"example.com", {{{"GET", hdlrs["GET"]}}, {"/var/www/html", {}, {}, oneMB, false, {}}}},
                   {"test.com",
                    {{{"GET", hdlrs["GET"]}, {"POST", hdlrs["POST"]}, {"DELETE", hdlrs["DELETE"]}},
                     {"/var/www/secure", {"index.html", "index.htm"}, {}, oneMB, false, {}}}},
                   {"test2.com", {{{"GET", hdlrs["GET"]}}, {"/usr/share/nginx/html", {}, {}, oneMB, false, {}}}}};

    return Router(hdlrs, defaultSvr, svrs, svrToLocs, uriToRoutes);
}
