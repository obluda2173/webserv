#include "test_stubs.h"
#include <Router.h>

Router newRouterTest() {
    std::string defaultSvr;
    std::set<std::string> svrs;
    std::map<std::string, std::set<std::string>> svrToLocs;
    std::map<std::string, std::string> routeToRoot;
    std::map<std::string, std::set<std::string>> routeToAllowedMethods;

    defaultSvr = "example.com";

    svrs = {"example.com", "test.com", "www.test.com", "test2.com", "test3.com"};

    svrToLocs = {
        {"example.com", {"/images/", "/css/scripts/", "/css/", "/css/styles/"}},
        {"test.com", {"/css/", "/js/", "/images/"}},
        {"www.test.com", {"/css/", "/js/", "/images/"}},
        {"test2.com", {}},
        {"test3.com", {"/"}},

    };

    routeToRoot = {
        {"example.com", "/var/www/html"},       {"example.com/images/", "/data"},           {"example.com/css/scripts/", "/data/scripts"},
        {"example.com/css/", "/data/static"},   {"example.com/css/styles/", "/data/extra"},

        {"test.com", "/var/www/secure"},        {"test.com/css/", "/data/static"},          {"test.com/js/", "/data/scripts"},
        {"test.com/images/", "/data2"},         {"www.test.com", "/var/www/secure"},        {"www.test.com/cs/", "/data/static"},
        {"www.test.com/js/", "/data/scripts"},  {"www.test.com/images/", "/data2"},

        {"test2.com", "/usr/share/nginx/html"}, {"test3.com", "/to/be/overwritten"},        {"test3.com/", "/test3/www/html"},

    };

    std::map<std::string, IHandler*> hdlrs = {
        {"GET", new StubHandler("GET")},
        {"POST", new StubHandler("POST")},
        {"DELETE", new StubHandler("DELETE")},
    };

    std::map<std::string, Route> routeToRoutes;
    routeToRoutes = {{"example.com", {{{"GET", hdlrs["GET"]}}, {"/var/www/html"}}},
                     {"test.com", {{{"GET", hdlrs["GET"]}, {"POST", hdlrs["POST"]}, {"DELETE", hdlrs["DELETE"]}}, {"/var/www/secure"}}},
                     {"test2.com", {{{"GET", hdlrs["GET"]}}, {"/usr/share/nginx/html"}}}};

    return Router(hdlrs, defaultSvr, routeToRoot, svrs, svrToLocs, routeToRoutes);
}
