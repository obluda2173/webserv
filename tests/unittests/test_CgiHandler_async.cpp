#include "CgiHandler.h"
#include "Connection.h"
#include "HttpRequest.h"
#include <gtest/gtest.h>
#include <thread>

HttpRequest createRequest(const std::string& method, const std::string& uri) {
    HttpRequest req;
    req.method = method;
    req.uri = uri;
    req.version = "HTTP/1.1";
    return req;
}

RouteConfig createConfig(bool enableCgi = true) {
    RouteConfig cfg;
    cfg.root = "tests/unittests/test_cgi/scripts";
    if (enableCgi)
        cfg.cgi["sh"] = "/bin/sh";
    return cfg;
}

TEST(CgiHandlerTestAsync, firstTest) {
    CgiHandler cgiHandler;

    int dummyFd = socket(AF_INET, SOCK_STREAM, 0);
    Connection* cgiConn = new Connection({}, dummyFd, NULL);
    HttpRequest cgiRequest = createRequest("GET", "/test_slow_script.sh");
    RouteConfig cgiConfig = createConfig(true);

    // Start timing
    auto startTime = std::chrono::steady_clock::now();
    // Handle the CGI request
    cgiHandler.handle(cgiConn, cgiRequest, cgiConfig);

    // Get the current time after CGI handler returns
    auto cgiHandlerReturnTime = std::chrono::steady_clock::now();
    auto cgiHandlerDuration =
        std::chrono::duration_cast< std::chrono::milliseconds >(cgiHandlerReturnTime - startTime).count();

    // The CGI handler should return quickly without waiting for the script to complete
    EXPECT_LT(cgiHandlerDuration, 1000) << "CGI handler took too long to return, it might be blocking";

    // sleep for a few milliseconds
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // Check that the connection is in a state that indicates ongoing processing
    EXPECT_TRUE(cgiConn->_response.body != NULL) << "CGI handler did not set a body provider";

    // Now wait a bit to let the CGI script complete
    std::this_thread::sleep_for(std::chrono::seconds(4));
}
