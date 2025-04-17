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
    // std::cout << want.body << " | " << got.body << std::endl;
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
        uri += "a/"; // Each segment adds 2 characters: "a/"
    }
    return uri;
}

INSTANTIATE_TEST_SUITE_P(
    ChunkedRequests,
    TestHttpParser,
    ::testing::Values(
        // 0 Valid request with Content-Length
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

        // 1 Invalid new line
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

        // 2 DoS attack
        TestHttpParserParams{
            10,
            0,
            1,
            {
                "POST", 
                generateLongURI(10000),
                "HTTP/1.1",
                {{"host", "localhost"}, {"content-length", "13"}},
                true
            },
            "POST " + generateLongURI(10000) + " HTTP/1.1\r\n"
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
        }
    )
);