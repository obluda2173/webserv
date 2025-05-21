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

TEST(CgiHandlerTestAsync, NonBlockingCgiExecution) {
    CgiHandler cgiHandler;
    int dummyFd = socket(AF_INET, SOCK_STREAM, 0);
    Connection* cgiConn = new Connection({}, dummyFd, "", NULL);
    HttpRequest cgiRequest = createRequest("GET", "/test_slow_script.sh");
    RouteConfig cgiConfig = createConfig(true);

    auto startTime = std::chrono::steady_clock::now();
    cgiHandler.handle(cgiConn, cgiRequest, cgiConfig);
    auto handlerReturnTime = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast< std::chrono::milliseconds >(handlerReturnTime - startTime).count();
    EXPECT_LT(duration, 100) << "CGI handler is blocking!";

    bool cgiCompleted = false;
    auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (std::chrono::steady_clock::now() < timeout) {
        if (cgiConn->getState() == Connection::SendResponse) {
            cgiCompleted = true;
            break;
        }
        cgiHandler.handleCgiProcess(cgiConn);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    EXPECT_TRUE(cgiCompleted) << "CGI processing timed out";
    EXPECT_NE(cgiConn->_response.body, nullptr) << "Response body not set";
    EXPECT_EQ(cgiConn->_response.statusCode, 200) << "Unexpected status code";
    delete cgiConn;
}
