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
    StubLogger logger;
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
// // 35 Header with multiple Cookie values
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
    "\r\n"},
// 39 Wrong method
TestHttpParserParams{   5,
    0,
    1,
    {},
    "GAT /file.txt HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Range: bytes=0-499;q=0.8, 500-999;q=0.5\r\n"
    "\r\n"},
// 40 Wrong version
TestHttpParserParams{5,
    0,
    1,
    {},
    "GET /file.txt HTTP/3.0\r\n"
    "Host: localhost\r\n"
    "Range: bytes=0-499;q=0.8, 500-999;q=0.5\r\n"
    "\r\n"},
// THIS IS THE NEW TEST CASES
// 41 Invalid version: missing "HTTP/"
TestHttpParserParams{   5,
    0,
    1,
    {},
    "GET /index.html HTTX/1.1\r\n"
    "Host: localhost\r\n"
    "\r\n"},
// 42 Invalid version: missing dot separator
TestHttpParserParams{   5,
    0,
    1,
    {},
    "GET /index.html HTTP/11\r\n"
    "Host: localhost\r\n"
    "\r\n"},
// 43 Invalid version: incomplete version string
TestHttpParserParams{   5,
    0,
    1,
    {},
    "GET /index.html HTTP/1\r\n"
    "Host: localhost\r\n"
    "\r\n"},
// 44 Invalid version: letters in version number
TestHttpParserParams{44,
    0,
    1,
    {},
    "GET /index.html HTTP/A.B\r\n"
    "Host: localhost\r\n"
    "\r\n"},
// 45 Invalid version: extra character in version
TestHttpParserParams{45,
    0,
    1,
    {},
    "GET /index.html HTTP/1.11\r\n"
    "Host: localhost\r\n"
    "\r\n"},
// 46 Invalid version: unsupported HTTP version (e.g., HTTP/0.9)
TestHttpParserParams{46,
    0,
    1,
    {},
    "GET /index.html HTTP/0.9\r\n"
    "Host: localhost\r\n"
    "\r\n"},
// 47 Invalid version: completely missing version
TestHttpParserParams{47,
    0,
    1,
    {},
    "GET /index.html \r\n"
    "Host: localhost\r\n"
    "\r\n"},
// 48 Invalid version: correct prefix but invalid format
TestHttpParserParams{48,
    0,
    1,
    {},
    "GET /index.html HTTP/abc\r\n"
    "Host: localhost\r\n"
    "\r\n"},
// 49 Invalid version: lowercase "http/" (case-sensitive check)
TestHttpParserParams{49,
    0,
    1,
    {},
    "GET /index.html http/1.1\r\n"
    "Host: localhost\r\n"
    "\r\n"},
// 50 Invalid host: empty string
TestHttpParserParams{50,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: \r\n"
    "\r\n"},
// 51 Invalid host: missing host before port
TestHttpParserParams{51,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: :8080\r\n"
    "\r\n"},
// 52 Invalid host: missing port after colon
TestHttpParserParams{52,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost:\r\n"
    "\r\n"},
// 53 Invalid host: port contains letters
TestHttpParserParams{53,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost:abc\r\n"
    "\r\n"},
// 54 Invalid host: port with symbols
TestHttpParserParams{54,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost:80#1\r\n"
    "\r\n"},
// 55 Invalid host: only whitespace
TestHttpParserParams{55,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host:    \r\n"
    "\r\n"},
// 56 Invalid host: colon but both sides empty
TestHttpParserParams{56,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: :\r\n"
    "\r\n"},
// 57 Invalid Content-Length: negative value
TestHttpParserParams{57,
    0,
    1,
    {},
    "GET / HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Content-Length: -10\r\n"
    "\r\n"},
// 58 Invalid Content-Length: non-numeric
TestHttpParserParams{58,
    0,
    1,
    {},
    "GET / HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Content-Length: abc\r\n"
    "\r\n"},
// 59 Invalid Content-Length: float value
TestHttpParserParams{59,
    0,
    1,
    {},
    "GET / HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Content-Length: 12.5\r\n"
    "\r\n"},
// 60 Invalid Content-Length: empty value
TestHttpParserParams{60,
    0,
    1,
    {},
    "GET / HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Content-Length:\r\n"
    "\r\n"},
// 61 Invalid Content-Length: trailing junk
TestHttpParserParams{61,
    0,
    1,
    {},
    "GET / HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Content-Length: 10abc\r\n"
    "\r\n"},
// 62 Invalid Content-Length: duplicated field (conflict)
TestHttpParserParams{62,
    0,
    1,
    {},
    "GET / HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Content-Length: 10\r\n"
    "Content-Length: 15\r\n"
    "\r\n"},
// 63 Invalid Transfer-Encoding: not chunked
TestHttpParserParams{63,
    0,
    1,
    {},
    "GET / HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Transfer-Encoding: gzip\r\n"
    "\r\n"},
// 64 Invalid Transfer-Encoding: incomplete "chunked"
TestHttpParserParams{64,
    0,
    1,
    {},
    "GET / HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Transfer-Encoding: chunk\r\n"
    "\r\n"},
// 65 Invalid Transfer-Encoding: case-sensitive mismatch
TestHttpParserParams{65,
    0,
    1,
    {},
    "GET / HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Transfer-Encoding: Chunked\r\n"
    "\r\n"},
// 66 Invalid Transfer-Encoding: ends with something else
TestHttpParserParams{66,
    0,
    1,
    {},
    "GET / HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Transfer-Encoding: chunked, gzip\r\n"
    "\r\n"},
// 67 Invalid Transfer-Encoding: empty value
TestHttpParserParams{67,
    0,
    1,
    {},
    "GET / HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Transfer-Encoding:\r\n"
    "\r\n"},
// 68 Invalid Transfer-Encoding: whitespace after "chunked"
TestHttpParserParams{68,
    0,
    1,
    {},
    "GET / HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Transfer-Encoding: chunked \r\n"
    "\r\n"},
// 69 Invalid Connection header: case-sensitive mismatch
TestHttpParserParams{69,
    0,
    1,
    {},
    "GET / HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Connection: Keep-Alive\r\n"
    "\r\n"},
// 70 Invalid Connection header: missing hyphen
TestHttpParserParams{70,
    0,
    1,
    {},
    "GET / HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Connection: keepalive\r\n"
    "\r\n"},
// 71 Invalid Connection header: trailing whitespace
TestHttpParserParams{71,
    0,
    1,
    {},
    "GET / HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Connection: close \r\n"
    "\r\n"},
// 72 Invalid Connection header: unsupported token
TestHttpParserParams{72,
    0,
    1,
    {},
    "GET / HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Connection: upgrade\r\n"
    "\r\n"},
// 73 Invalid Connection header: multiple tokens
TestHttpParserParams{73,
    0,
    1,
    {},
    "GET / HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Connection: keep-alive, close\r\n"
    "\r\n"},
// 74 Invalid Connection header: empty value
TestHttpParserParams{74,
    0,
    1,
    {},
    "GET / HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Connection:\r\n"
    "\r\n"},
// 75 Invalid Content-Type: Missing subtype
TestHttpParserParams{75,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Content-Type: text/\r\n"
    "\r\n"},
// 76 Invalid Content-Type: Unknown type
TestHttpParserParams{76,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Content-Type: xyz/html\r\n"
    "\r\n"},
// 77 Invalid Content-Type: Missing slash
TestHttpParserParams{77,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Content-Type: texthtml\r\n"
    "\r\n"},
// 78 Invalid Content-Type: Empty charset
TestHttpParserParams{78,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Content-Type: text/html; charset=\r\n"
    "\r\n"},
// 79 Invalid Content-Type: multipart without boundary
TestHttpParserParams{79,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Content-Type: multipart/form-data\r\n"
    "\r\n"},
// 80 Invalid Content-Type: multipart with empty boundary
TestHttpParserParams{80,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Content-Type: multipart/form-data; boundary=\r\n"
    "\r\n"},
// 81 Invalid Accept header: missing slash in media type
TestHttpParserParams{81,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Accept: textplain\r\n"
    "\r\n"},
// 82 Invalid Accept header: slash at the start
TestHttpParserParams{82,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Accept: /plain\r\n"
    "\r\n"},
// 83 Invalid Accept header: slash at the end
TestHttpParserParams{83,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Accept: text/\r\n"
    "\r\n"},
// 84 Invalid Accept header: unknown type "xyz"
TestHttpParserParams{84,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Accept: xyz/plain\r\n"
    "\r\n"},
// 85 Invalid Accept header: invalid charset
TestHttpParserParams{85,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Accept: text/html; charset=@utf-8\r\n"
    "\r\n"},
// 86 Invalid Accept header: invalid boundary
TestHttpParserParams{86,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Accept: multipart/form-data; boundary=!!badboundary\r\n"
    "\r\n"},
// 87 Invalid Accept header: non-numeric q-value
TestHttpParserParams{87,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Accept: text/html;q=abc\r\n"
    "\r\n"},
// 88 Invalid Accept header: q-value greater than 1.0
TestHttpParserParams{88,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Accept: text/html;q=1.5\r\n"
    "\r\n"},
// 89 Invalid Accept header: negative q-value
TestHttpParserParams{89,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Accept: text/html;q=-0.2\r\n"
    "\r\n"},
// 90 Invalid Accept-Encoding header: unknown encoding "foobar"
TestHttpParserParams{90,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Accept-Encoding: foobar\r\n"
    "\r\n"},

// 91 Invalid Accept-Encoding header: malformed q-value
TestHttpParserParams{91,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Accept-Encoding: gzip;q=abc\r\n"
    "\r\n"},

// 92 Invalid Accept-Encoding header: q-value greater than 1
TestHttpParserParams{92,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Accept-Encoding: br;q=1.5\r\n"
    "\r\n"},

// 93 Invalid Accept-Encoding header: negative q-value
TestHttpParserParams{93,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Accept-Encoding: deflate;q=-0.1\r\n"
    "\r\n"},

// 94 Invalid Accept-Encoding header: empty value
TestHttpParserParams{94,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Accept-Encoding: \r\n"
    "\r\n"},
// 95 Invalid Accept-Encoding header: whitespace after encoding
TestHttpParserParams{95,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Accept-Encoding: gzip \r\n"
    "\r\n"},
// 96 Invalid Accept-Language: empty value
TestHttpParserParams{96,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Accept-Language: \r\n"
    "\r\n"},

// 97 Invalid Accept-Language: malformed tag with leading '-'
TestHttpParserParams{97,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Accept-Language: -US\r\n"
    "\r\n"},

// 98 Invalid Accept-Language: malformed tag with trailing '-'
TestHttpParserParams{98,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Accept-Language: en-\r\n"
    "\r\n"},

// 99 Invalid Accept-Language: language code not 2 letters
TestHttpParserParams{99,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Accept-Language: eng-US\r\n"
    "\r\n"},

// 100 Invalid Accept-Language: invalid q-value format
TestHttpParserParams{100,
     0,
     1,
     {},
     "GET /index.html HTTP/1.1\r\n"
     "Host: localhost\r\n"
     "Accept-Language: en-US;q=abc\r\n"
     "\r\n"},

// 101 Invalid Accept-Language: q-value greater than 1
TestHttpParserParams{101,
     0,
     1,
     {},
     "GET /index.html HTTP/1.1\r\n"
     "Host: localhost\r\n"
     "Accept-Language: en;q=1.5\r\n"
     "\r\n"},

// 102 Invalid Accept-Language: negative q-value
TestHttpParserParams{102,
     0,
     1,
     {},
     "GET /index.html HTTP/1.1\r\n"
     "Host: localhost\r\n"
     "Accept-Language: fr;q=-0.1\r\n"
     "\r\n"},
// 103 Invalid Cookie: empty value
TestHttpParserParams{103,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Cookie: \r\n"
    "\r\n"},

// 104 Invalid Cookie: missing '='
TestHttpParserParams{104,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Cookie: sessionid\r\n"
    "\r\n"},

// 105 Invalid Cookie: '=' at beginning
TestHttpParserParams{105,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Cookie: =value\r\n"
    "\r\n"},

// 106 Invalid Cookie: '=' at end
TestHttpParserParams{106,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Cookie: sessionid=\r\n"
    "\r\n"},

// 107 Invalid Cookie: empty key
TestHttpParserParams{107,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Cookie: =abc123\r\n"
    "\r\n"},
// 108 Invalid Range: missing "bytes="
TestHttpParserParams{108,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Range: 500-1000\r\n"
    "\r\n"},

// 109 Invalid Range: missing start or end in range
TestHttpParserParams{109,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Range: bytes=-500\r\n"
    "\r\n"},

// 110 Invalid Range: missing dash in range
TestHttpParserParams{110,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Range: bytes=500\r\n"
    "\r\n"},

// 111 Invalid Range: dash at start or end of range
TestHttpParserParams{111,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Range: bytes=-500-\r\n"
    "\r\n"},

// 112 Invalid Range: non-numeric value in range
TestHttpParserParams{112,
    0,
    1,
    {},
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Range: bytes=500-abc\r\n"
    "\r\n"},
// ADDITIONAL TEST CASES
// 113 Invalid Version: not digit at the beginning (e.g., "HTTP/a.1")
TestHttpParserParams{113,
    0,
    1,
    {},
    "GET /index.html HTTP/a.1\r\n"
    "Host: localhost\r\n"
    "\r\n"},
// 114 Valid Accept-Language: invalid q-value format (space after equal sign)
TestHttpParserParams{114,
    1,
    0,
    {
        "GET",
        "/index.html",
        "HTTP/1.1",
        {{"host", "localhost"}, {"accept-language", "en-US;q= 0.5"}}
    },
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Accept-Language: en-US;q= 0.5\r\n"
    "\r\n"},
// 115 Valid Accept-Language: invalid q-value format (space before equal sign)
TestHttpParserParams{115,
    1,
    0,
    {
        "GET",
        "/index.html",
        "HTTP/1.1",
        {{"host", "localhost"},{"accept-language", "en-US;q =0.5"}}
    },
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Accept-Language: en-US;q =0.5\r\n"
    "\r\n"}


        // TODO: added test from Kay
        // TestHttpParserParams{5,
        //                      0,
        //                      1,
        //                      {
        //                          "GET",
        //                          "/file.txt",
        //                          "HTTP/1.1",
        //                          {{"host", "localhost"}, {"range", "bytes=0-499, 500-999"}},
        //                      },
        //                      "GET /file.txt   HTTP/1.1  \r\n"
        //                      "Host: localhost\r\n"
        //                      "Range: bytes=0-499, 500-999\r\n"
        //                      "\r\n"}
        ));
