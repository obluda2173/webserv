#include "HttpParser.h"
#include "HttpRequest.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

struct TestHttpParserParams {
    int bufLen;                  // Buffer size for splitting input
    int wantReady;               // Should parser be ready?
    int wantError;               // Should parser be in error state?
    HttpRequest wantReq;         // Expected parsed request
    std::string req;             // Raw HTTP request (with \r\n)
};

class TestHttpParser : public ::testing::TestWithParam<TestHttpParserParams> {};

void assertEqualHttpRequest(const HttpRequest& want, const HttpRequest& got) {
    // std::cout << "----------------------------------------------------\n";
    // std::cout << want.method << " | " << got.method << std::endl;
    // std::cout << want.uri << " | " << got.uri << std::endl;
    // std::cout << want.version << " | " << got.version << std::endl;
    // std::cout << "----------------------------------------------------\n";
    EXPECT_EQ(want.method, got.method);
    EXPECT_EQ(want.uri, got.uri);
    EXPECT_EQ(want.version, got.version);
    EXPECT_EQ(want.headers, got.headers);
    EXPECT_EQ(want.hasBody, got.hasBody);
}

TEST_P(TestHttpParser, Parsing) {
    HttpParser parser;
    const auto& params = GetParam();

    // Split request into chunks to simulate partial data
    for (size_t i = 0; i < params.req.size(); i += params.bufLen) {
        size_t chunkSize = std::min<size_t>(params.bufLen, params.req.size() - i);
        parser.feed(params.req.data() + i, chunkSize);
    }

    EXPECT_EQ(params.wantError, parser.error());
    EXPECT_EQ(params.wantReady, parser.ready());
    
    if (parser.ready()) {
        assertEqualHttpRequest(params.wantReq, parser.getRequest());
    }
}

std::string generateLongURI(size_t segmentCount) {
    std::string uri = "/";
    for (size_t i = 0; i < segmentCount; ++i) {
        uri += "a/";
    }
    return uri;
}

std::string generateManyHeaders(size_t headersCount) {
    std::string uri;
    for (size_t i = 0; i < headersCount; ++i) {
        uri += "Host: localhost" + std::to_string(i) + "\r\n";
    }
    return uri;
}

INSTANTIATE_TEST_SUITE_P(
    ChunkedRequests,
    TestHttpParser,
    ::testing::Values(
        // 0 Average request
        TestHttpParserParams{
            10,
            1,
            0,
            {
                "POST", "/upload", "HTTP/1.1",
                {{"host", "localhost"}, {"content-length", "13"}},
                true
            },
            "POST /upload HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello, World!"
        },

        // 1 Invalid new lines
        TestHttpParserParams{
            10,
            0,
            1,
            {
                "POST", "/upload", "HTTP/1.1",
                {{"host", "localhost"}, {"content-length", "13"}},
                true
            },
            "POST /upload HTTP/1.1\n"
            "Host: localhost\n"
            "Content-Length: 13\n"
            "\n"
            "Hello, World!"
        },

        // 2 Maximum URI length (DoS attack)
        TestHttpParserParams{
            10,
            0,
            1,
            {},
            "POST " + generateLongURI(1000000) + " HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello, World!"
        },

        // 3 Valid start-line white spaces
        TestHttpParserParams{
            10,
            1,
            0,
            {
                "POST", "/upload", "HTTP/1.1",
                {{"host", "localhost"}, {"content-length", "13"}},
                true
            },
            "POST   \t  /upload   \t\t     HTTP/1.1 \t\t \r\n"
            "Host: localhost\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello, World!"
        },

        // 4 Invalid white space
        TestHttpParserParams{
            10,
            0,
            1,
            {
                "POST", "/upload", "HTTP/1.1",
                {{"host", "localhost"}, {"content-length", "13"}},
                true
            },
            "POST /upload HTTP/1.1\r\n  \r\n"
            "Host: localhost\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello, World!"
        },

        // 5 Invalid URI
        TestHttpParserParams{
            10,
            0,
            1,
            {},
            "GET /some path/with space HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello, World!"
        },

        // 6 Malformed Request Line
        TestHttpParserParams{
            10,
            0,
            1,
            {},
            "POST /upload\r\n"
            "Host: localhost\r\n"
            "\r\n"
        },

        // 7 Header with No Value
        TestHttpParserParams{
            10,
            1,
            0,
            {
                "GET", "/", "HTTP/1.1", 
                {{"host", ""}}, 
                false
            },
            "GET / HTTP/1.1\r\n"
            "Host:\r\n"
            "\r\n"
        },

        // 8 Duplicate Headers
        TestHttpParserParams{
            10,
            1,
            0,
            {
                "GET", "/", "HTTP/1.1",
                {{"set-cookie", "sessionId=123"}, {"set-cookie", "userId=456"}},
                false
            },
            "GET / HTTP/1.1\r\n"
            "Set-Cookie: sessionId=123\r\n"
            "Set-Cookie: userId=456\r\n"
            "\r\n"
        },

        // 9 
        TestHttpParserParams{
            10,
            1,
            0,
            {
                "GET", "/", "HTTP/1.1", 
                {{"custom-header", "value_with_special_chars!@#$%"}},
                false
            },
            "GET / HTTP/1.1\r\n"
            "Custom-Header: value_with_special_chars!@#$%\r\n"
            "\r\n"
        },

        // 10 Maximum Header Count (DoS Protection)
        TestHttpParserParams{
            10,
            0,
            1,
            {},
            "GET / HTTP/1.1\r\n" + generateManyHeaders(1000000) + "\r\n"
        },

        // 11 Pipelined Requests
        TestHttpParserParams{
            10,
            1,
            0,
            {
                "GET", "/first", "HTTP/1.1",
                {{"host", "localhost"}},
                false
            },
            "GET /first HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "\r\n"
            "GET /second HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "\r\n"
        },

        // 12 URI with Query Parameters
        TestHttpParserParams{
            10,
            1,
            0,
            {
                "GET", "/search?q=test&lang=en", "HTTP/1.1",
                {{"host", "localhost"}},
                false
            },
            "GET /search?q=test&lang=en HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "\r\n"
        },

        // 13 Header with Leading Whitespace
        TestHttpParserParams{
            10,
            1,
            0,
            {
                "GET", "/", "HTTP/1.1",
                {{"host", "localhost"}},
                false
            },
            "GET / HTTP/1.1\r\n"
            "Host:    localhost\r\n"
            "\r\n"
        },

        // 14 Request with Asterix URI
        TestHttpParserParams{
            5,
            1,
            0,
            {
                "OPTIONS", "*", "HTTP/1.1",
                {{"host", "localhost"}},
                false
            },
            "OPTIONS * HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "\r\n"
        },

        // 15 Duplicate Content-Length Headers
        TestHttpParserParams{
            5,
            1,
            0,
            {
                "POST", "/upload", "HTTP/1.1",
                {{"host", "localhost"}, {"content-length", "5"}, {"content-length", "10"}},
                true
            },
            "POST /upload HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "Content-Length: 5\r\n"
            "Content-Length: 10\r\n"
            "\r\n"
            "Hello\r\n"
        },

        // 16 Missing Space in Request Line
        TestHttpParserParams{
            5,
            0,
            1,
            {},
            "GET/ HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "\r\n"
        },

        // 17 URI with Encoded Characters
        TestHttpParserParams{
            5,
            1,
            0,
            {
                "GET", "/path%20with%20spaces", "HTTP/1.1",
                {{"host", "localhost"}},
                false
            },
            "GET /path%20with%20spaces HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "\r\n"
        },

        // 18 Very Long Header Name
        TestHttpParserParams{
            5,
            1,
            0,
            {
                "GET", "/", "HTTP/1.1",
                {{"x-very-long-header-name-that-goes-on-and-on", "value"}},
                false
            },
            "GET / HTTP/1.1\r\n"
            "X-Very-Long-Header-Name-That-Goes-On-And-On: value\r\n"
            "\r\n"
        }
    )
);