#include "HttpParser.h"
#include "HttpRequest.h"
#include "test_stubs.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

struct TestHttpParserParams {
    int bufLen;          // Buffer size for splitting input
    int wantReady;       // Should parser be ready?
    int wantError;       // Should parser be in error state?
    HttpRequest wantReq; // Expected parsed request
    std::string req;     // Raw HTTP request (with \r\n)
};

class TestHttpParser : public ::testing::TestWithParam<TestHttpParserParams> {};

void assertEqualHttpRequest(const HttpRequest& want, const HttpRequest& got) {
    EXPECT_EQ(want.method, got.method);
    EXPECT_EQ(want.uri, got.uri);
    EXPECT_EQ(want.version, got.version);
    EXPECT_EQ(want.headers, got.headers);
}

TEST_P(TestHttpParser, Parsing) {
    ILogger* logger = new StubLogger();
    HttpParser parser(logger);
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
    delete logger;
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
    ChunkedRequests, TestHttpParser,
    ::testing::Values(
        // 0 Average request
        TestHttpParserParams{10,
                             1,
                             0,
                             {
                                 "POST",
                                 "/upload",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"content-length", "13"}},
                             },
                             "POST /upload HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Content-Length: 13\r\n"
                             "\r\n"
                             "Hello, World!"},

        // 1 Invalid new lines
        TestHttpParserParams{10,
                             0,
                             1,
                             {
                                 "POST",
                                 "/upload",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"content-length", "13"}},
                             },
                             "POST /upload HTTP/1.1\n"
                             "Host: localhost\n"
                             "Content-Length: 13\n"
                             "\n"
                             "Hello, World!"},

        // 2 Maximum URI length (DoS attack)
        TestHttpParserParams{10,
                             0,
                             1,
                             {},
                             "POST " + generateLongURI(1000000) +
                                 " HTTP/1.1\r\n"
                                 "Host: localhost\r\n"
                                 "Content-Length: 13\r\n"
                                 "\r\n"
                                 "Hello, World!"},

        // 3 Valid start-line white spaces
        TestHttpParserParams{10,
                             1,
                             0,
                             {
                                 "POST",
                                 "/upload",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"content-length", "13"}},
                             },
                             "POST   \t  /upload   \t\t     HTTP/1.1 \t\t \r\n"
                             "Host: localhost\r\n"
                             "Content-Length: 13\r\n"
                             "\r\n"
                             "Hello, World!"},

        // 4 Invalid white space
        TestHttpParserParams{10,
                             0,
                             1,
                             {
                                 "POST",
                                 "/upload",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"content-length", "13"}},
                             },
                             "POST /upload HTTP/1.1\r\n  \r\n"
                             "Host: localhost\r\n"
                             "Content-Length: 13\r\n"
                             "\r\n"
                             "Hello, World!"},

        // 5 Invalid URI
        TestHttpParserParams{10,
                             0,
                             1,
                             {},
                             "GET /some path/with space HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Content-Length: 13\r\n"
                             "\r\n"
                             "Hello, World!"},

        // 6 Malformed Request Line
        TestHttpParserParams{10,
                             0,
                             1,
                             {},
                             "POST /upload\r\n"
                             "Host: localhost\r\n"
                             "\r\n"},

        // 7 Header with No Value
        TestHttpParserParams{10,
                             0,
                             1,
                             {
                                 "GET",
                                 "/",
                                 "HTTP/1.1",
                                 {{"host", ""}},
                             },
                             "GET / HTTP/1.1\r\n"
                             "Host:\r\n"
                             "\r\n"},

        // 8 Duplicate Headers
        TestHttpParserParams{10,
                             0,
                             1,
                             {},
                             "GET / HTTP/1.1\r\n"
                             "Set-Cookie: sessionId=123\r\n"
                             "Set-Cookie: userId=456\r\n"
                             "\r\n"},

        // 9 Header with Special Characters
        TestHttpParserParams{10,
                             1,
                             0,
                             {
                                 "GET",
                                 "/",
                                 "HTTP/1.1",
                                 {{"custom-header", "value_with_special_chars!@#$%"}},
                             },
                             "GET / HTTP/1.1\r\n"
                             "Custom-Header: value_with_special_chars!@#$%\r\n"
                             "\r\n"},

        // 10 Maximum Header Count (DoS Protection)
        TestHttpParserParams{10, 0, 1, {}, "GET / HTTP/1.1\r\n" + generateManyHeaders(1000000) + "\r\n"},

        // 11 Pipelined Requests
        TestHttpParserParams{10,
                             1,
                             0,
                             {
                                 "GET",
                                 "/first",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}},
                             },
                             "GET /first HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "\r\n"
                             "GET /second HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "\r\n"},

        // 12 URI with Query Parameters
        TestHttpParserParams{10,
                             1,
                             0,
                             {
                                 "GET",
                                 "/search?q=test&lang=en",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}},
                             },
                             "GET /search?q=test&lang=en HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "\r\n"},

        // 13 Header with Leading Whitespace
        TestHttpParserParams{10,
                             1,
                             0,
                             {
                                 "GET",
                                 "/",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}},
                             },
                             "GET / HTTP/1.1\r\n"
                             "Host:    localhost\r\n"
                             "\r\n"},

        // 14 Request with Asterix URI
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "OPTIONS",
                                 "*",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}},
                             },
                             "OPTIONS * HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "\r\n"},

        // 15 Duplicate Content-Length Headers
        TestHttpParserParams{5,
                             0,
                             1,
                             {},
                             "POST /upload HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Content-Length: 5\r\n"
                             "Content-Length: 10\r\n"
                             "\r\n"
                             "Hello\r\n"},

        // 16 Missing Space in Request Line
        TestHttpParserParams{5,
                             0,
                             1,
                             {},
                             "GET/ HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "\r\n"},

        // 17 URI with Encoded Characters
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "GET",
                                 "/path%20with%20spaces",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}},
                             },
                             "GET /path%20with%20spaces HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "\r\n"},

        // 18 Very Long Header Name
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "GET",
                                 "/",
                                 "HTTP/1.1",
                                 {{"x-very-long-header-name-that-goes-on-and-on", "value"}},
                             },
                             "GET / HTTP/1.1\r\n"
                             "X-Very-Long-Header-Name-That-Goes-On-And-On: value\r\n"
                             "\r\n"},

        // 19 Header Host with Port
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "GET",
                                 "/",
                                 "HTTP/1.1",
                                 {{"host", "localhost:8080"}},
                             },
                             "GET / HTTP/1.1\r\n"
                             "Host: localhost:8080\r\n"
                             "\r\n"},
        // 20 Header with Content-length and Transfer-Encoding
        TestHttpParserParams{5,
                             0,
                             1,
                             {},
                             "POST /upload HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Content-Length: 5\r\n"
                             "Transfer-Encoding: chunked\r\n"
                             "\r\n"},
        // 21 Header with Content-length only
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "POST",
                                 "/upload",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"content-length", "5"}},
                             },
                             "POST /upload HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Content-Length: 5\r\n"
                             "\r\n"},
        // 22 Header with Transfer-Encoding only
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "POST",
                                 "/upload",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"transfer-encoding", "chunked"}},
                             },
                             "POST /upload HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Transfer-Encoding: chunked\r\n"
                             "\r\n"},
        // 23 Header with Connection: keep-alive
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "GET",
                                 "/",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"connection", "keep-alive"}},
                             },
                             "GET / HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Connection: keep-alive\r\n"
                             "\r\n"},
        // 24 Header with Connection: close
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "GET",
                                 "/",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"connection", "close"}},
                             },
                             "GET / HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Connection: close\r\n"
                             "\r\n"},
        // 25 Header with Content-Type
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "POST",
                                 "/upload",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"content-type", "application/json"}},
                             },
                             "POST /upload HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Content-Type: application/json\r\n"
                             "\r\n"},
        // 26 Header with Accept
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "GET",
                                 "/",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"accept", "application/json"}},
                             },
                             "GET / HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Accept: application/json\r\n"
                             "\r\n"},
        // 27 Header with multiple Accept values
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "GET",
                                 "/",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"accept", "application/json, text/html"}},
                             },
                             "GET / HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Accept: application/json, text/html\r\n"
                             "\r\n"},
        // 28 Header with Accept-Encoding
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "GET",
                                 "/",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"accept-encoding", "gzip, deflate"}},
                             },
                             "GET / HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Accept-Encoding: gzip, deflate\r\n"
                             "\r\n"},
        // 29 Header with multiple Accept-Encoding values
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "GET",
                                 "/",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"accept-encoding", "gzip, deflate, br"}},
                             },
                             "GET / HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Accept-Encoding: gzip, deflate, br\r\n"
                             "\r\n"},
        // 30 Header with multiple Accept-Encoding values and qualifiers
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "GET",
                                 "/",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"accept-encoding", "gzip;q=0.8, deflate;q=0.5"}},
                             },
                             "GET / HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Accept-Encoding: gzip;q=0.8, deflate;q=0.5\r\n"
                             "\r\n"},
        // 31 Header with Accept-Language
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "GET",
                                 "/",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"accept-language", "en-US, en;q=0.5"}},
                             },
                             "GET / HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Accept-Language: en-US, en;q=0.5\r\n"
                             "\r\n"},
        // 32 Header with multiple Accept-Language values
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "GET",
                                 "/",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"accept-language", "en-US, fr;q=0.8"}},
                             },
                             "GET / HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Accept-Language: en-US, fr;q=0.8\r\n"
                             "\r\n"},
        // 33 Header with multiple Accept-Language values and qualifiers
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "GET",
                                 "/",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"accept-language", "en-US;q=0.8, fr;q=0.5"}},
                             },
                             "GET / HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Accept-Language: en-US;q=0.8, fr;q=0.5\r\n"
                             "\r\n"},
        // 34 Header with Cookie
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "GET",
                                 "/",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"cookie", "sessionId=123; userId=456"}},
                             },
                             "GET / HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Cookie: sessionId=123; userId=456\r\n"
                             "\r\n"},
        // 35 Header with multiple Cookie values
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "GET",
                                 "/",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"cookie", "sessionId=123; userId=456; theme=dark"}},
                             },
                             "GET / HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Cookie: sessionId=123; userId=456; theme=dark\r\n"
                             "\r\n"},
        // 36 Header with Range
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "GET",
                                 "/file.txt",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"range", "bytes=0-499"}},
                             },
                             "GET /file.txt HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Range: bytes=0-499\r\n"
                             "\r\n"},
        // 37 Header with multiple Range values
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "GET",
                                 "/file.txt",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"range", "bytes=0-499, 500-999"}},
                             },
                             "GET /file.txt HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Range: bytes=0-499, 500-999\r\n"
                             "\r\n"},
        // 38 Header with multiple Range values and qualifiers
        TestHttpParserParams{5,
                             1,
                             0,
                             {
                                 "GET",
                                 "/file.txt",
                                 "HTTP/1.1",
                                 {{"host", "localhost"}, {"range", "bytes=0-499;q=0.8, 500-999;q=0.5"}},
                             },
                             "GET /file.txt HTTP/1.1\r\n"
                             "Host: localhost\r\n"
                             "Range: bytes=0-499;q=0.8, 500-999;q=0.5\r\n"
                             "\r\n"}));
