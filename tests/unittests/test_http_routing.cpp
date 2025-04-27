#include <gtest/gtest.h>
#include <fstream>
#include "Router.h"
#include "HttpParser.h"
#include "ConfigParser.h"
#include "Logger.h"

class RouterTest : public ::testing::Test {
protected:
    void TearDown() override {
        std::remove("testConfig.conf");
    }

    std::vector<ServerConfig> parseConfig(const std::string& content) {
        std::ofstream file("testConfig.conf");
        file << content;
        file.close();
        ConfigParser parser("testConfig.conf");
        return parser.getServersConfig();
    }

    HttpRequest parseRequest(const std::string& rawRequest, int chunkSize = 0) {
        Logger logger;
        HttpParser parser(logger);
        if (chunkSize <= 0) {
            parser.feed(rawRequest.data(), rawRequest.size());
        } else {
            for (size_t i = 0; i < rawRequest.size(); i += chunkSize) {
                size_t size = std::min<size_t>(chunkSize, rawRequest.size() - i);
                parser.feed(rawRequest.data() + i, size);
            }
        }
        if (!parser.ready()) {
            throw std::runtime_error("Failed to parse HTTP request");
        }
        return parser.getRequest();
    }
};

struct RouterTestParams {
    std::string config;           // Server configuration string
    std::string rawRequest;       // Raw HTTP request
    int port;                     // Port to match against
    std::string expectedServer;   // Expected server name (or "" if none)
    std::string expectedLocation; // Expected location prefix (or "" if none)
    bool expectThrow;             // Should the match throw an exception?
};

class RouterParameterizedTest : public RouterTest, public ::testing::WithParamInterface<RouterTestParams> {};

TEST_P(RouterParameterizedTest, MatchServerAndLocation) {
    const auto& params = GetParam();
    std::vector<ServerConfig> servers = parseConfig(params.config);
    Router router(servers);
    HttpRequest request = parseRequest(params.rawRequest, 10);

    if (params.expectThrow) {
        EXPECT_THROW(router.match(request, params.port), std::runtime_error);
    } else {
        auto [server, location] = router.match(request, params.port);
        if (params.expectedServer.empty()) {
            EXPECT_EQ(server, nullptr);
        } else {
            ASSERT_NE(server, nullptr);
            EXPECT_EQ(server->serverNames[0], params.expectedServer);
        }
        if (params.expectedLocation.empty()) {
            EXPECT_EQ(location, nullptr);
        } else {
            ASSERT_NE(location, nullptr);
            EXPECT_EQ(location->prefix, params.expectedLocation);
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
    RouterTests,
    RouterParameterizedTest,
    ::testing::Values(
        // Test 1: No server for port
        RouterTestParams{
            "server {\n"
            "    listen 80;\n"
            "    server_name example.com;\n"
            "    root /var/www/html;\n"
            "}\n",
            "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n",
            8080,
            "",
            "",
            true
        },
        // Test 2: Matching server and no location
        RouterTestParams{
            "server {\n"
            "    listen 80;\n"
            "    server_name example.com;\n"
            "    root /var/www/html;\n"
            "}\n",
            "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n",
            80,
            "example.com",
            "",
            false
        },
        // Test 3: Multiple servers, specific host match
        RouterTestParams{
            "server {\n"
            "    listen 80;\n"
            "    server_name example.com;\n"
            "    root /var/www/html;\n"
            "}\n"
            "server {\n"
            "    listen 80;\n"
            "    server_name test.com;\n"
            "    root /var/www/test;\n"
            "}\n",
            "GET / HTTP/1.1\r\nHost: test.com\r\n\r\n",
            80,
            "test.com",
            "",
            false
        },
        // Test 4: Location matching with longest prefix
        RouterTestParams{
            "server {\n"
            "    listen 80;\n"
            "    server_name example.com;\n"
            "    root /var/www/html;\n"
            "    location / {\n"
            "        root /var/www/root;\n"
            "    }\n"
            "    location /images/ {\n"
            "        root /var/www/images;\n"
            "    }\n"
            "    location /images/photos/ {\n"
            "        root /var/www/photos;\n"
            "    }\n"
            "}\n",
            "GET /images/photos/dog.jpg HTTP/1.1\r\nHost: example.com\r\n\r\n",
            80,
            "example.com",
            "/images/photos/",
            false
        },
        // Test 5: No matching location
        RouterTestParams{
            "server {\n"
            "    listen 80;\n"
            "    server_name example.com;\n"
            "    root /var/www/html;\n"
            "    location /images/ {\n"
            "        root /var/www/images;\n"
            "    }\n"
            "}\n",
            "GET /docs/report.pdf HTTP/1.1\r\nHost: example.com\r\n\r\n",
            80,
            "example.com",
            "",
            false
        }
    )
);
