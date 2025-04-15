#include "HttpParser.h"
#include "HttpRequest.h"
#include <gtest/gtest.h>
# include <gmock/gmock.h>

struct TestHttpParserParams {
    int bufLen;                  // Buffer size for splitting input
    int wantReady;               // Should parser be ready?
    int wantError;               // Should parser be in error state?
    HttpRequest wantReq;         // Expected parsed request
    std::string req;             // Raw HTTP request (with \r\n)
};

class TestHttpParser : public ::testing::TestWithParam<TestHttpParserParams> {};

void assertEqualHttpRequest(const HttpRequest& want, const HttpRequest& got) {
    std::cout << "----------------------------------------------------\n";
    std::cout << want.method << " | " << got.method << std::endl;
    std::cout << want.uri << " | " << got.uri << std::endl;
    std::cout << want.version << " | " << got.version << std::endl;
    std::cout << want.body << " | " << got.body << std::endl;
    std::cout << "----------------------------------------------------\n";
    EXPECT_EQ(want.method, got.method);
    EXPECT_EQ(want.uri, got.uri);
    EXPECT_EQ(want.version, got.version);
    EXPECT_EQ(want.headers, got.headers);
    EXPECT_EQ(want.body, got.body);
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

INSTANTIATE_TEST_SUITE_P(
    ChunkedRequests,
    TestHttpParser,
    ::testing::Values(
        // Valid request with Content-Length
        TestHttpParserParams{
            10,
            1,  // Expect ready
            0,  // No error
            {   // Expected request
                "POST", 
                "/upload", 
                "HTTP/1.1",
                {{"Host", "localhost"}, {"Content-Length", "13"}},
                "Hello, World!"  // Body
            },
            "POST /upload HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello, World!"
        },

        // Valid chunked request
        TestHttpParserParams{
            4,  // Split input into 4-byte chunks
            1,  // Expect ready
            0,  // No error
            {   // Expected request
                "POST", 
                "/upload", 
                "HTTP/1.1",
                {{"Host", "localhost"}, {"Transfer-Encoding", "chunked"}},
                "MozillaDeveloper"  // Combined chunks
            },
            "POST /upload HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "7\r\n"        // Chunk 1: 7 bytes
            "Mozilla\r\n"  // Data
            "9\r\n"        // Chunk 2: 9 bytes
            "Developer\r\n"
            "0\r\n"        // End of chunks
            "\r\n"
        },

        // Invalid chunk size (non-hex)
        TestHttpParserParams{
            3,
            0,
            1,  // Expect error
            {},
            "POST / HTTP/1.1\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "1g\r\n"  // Invalid hex
            "data\r\n"
        },

        // Unterminated chunk
        TestHttpParserParams{
            5,
            0,
            0,  // Not an error yet (waiting for data)
            {},
            "POST / HTTP/1.1\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "5\r\n"
            "Hello"  // Missing \r\n after data
        }
    )
);