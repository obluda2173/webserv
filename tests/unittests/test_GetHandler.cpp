#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "test_stubs.h"
#include "GetHandler.h"

struct TestGetHandlerParams {
    HttpRequest req;
    RouteConfig cfg;
    HttpResponse wantResp;
};

class TestGetHandler : public ::testing::TestWithParam<TestGetHandlerParams> {};

class RequestBuilder {
    private:
        HttpRequest req;

    public:
        RequestBuilder() {
            req.method = "GET";
            req.uri = "/test.txt";
            req.version = "HTTP/1.1";
            req.headers = {};
        }
        RequestBuilder& withMethod(const std::string& method) {
            req.method = method;
            return *this;
        }
        RequestBuilder& withUri(const std::string& uri) {
            req.uri = uri;
            return *this;
        }
        RequestBuilder& withHeader(const std::string& key, const std::string& value) {
            req.headers[key] = value;
            return *this;
        }
        HttpRequest build() const { return req; }
};

class RouteConfigBuilder {
    private:
        RouteConfig cfg;

    public:
        RouteConfigBuilder() {
            cfg.root = "./tests/unittests/test_root";
            cfg.index = {};
            cfg.clientMaxBody = 0;
            cfg.autoindex = false;
        }
        RouteConfigBuilder& withRoot(const std::string& root) {
            cfg.root = root;
            return *this;
        }
        RouteConfigBuilder& withAutoIndex(bool autoIndex) {
            cfg.autoindex = autoIndex;
            return *this;
        }
        RouteConfigBuilder& withErrorPage(std::map<int, std::string> errorPage) {
            cfg.errorPage = errorPage;
            return *this;
        }
        RouteConfigBuilder& withIndex(std::vector<std::string> index) {
            cfg.index = index;
            return *this;
        }
        RouteConfig build() const { return cfg; }
};

class ResponseBuilder {
    private:
        HttpResponse resp;

    public:
        ResponseBuilder() {
            resp.version = "HTTP/1.1";
            resp.statusCode = 400;
            resp.statusMessage = "Bad Request";
            resp.contentType = "text/plain";
            resp.contentLanguage = "en-US";
            resp.contentLength = 0;
            resp.body = nullptr;
            resp.isRange = false;
            resp.isClosed = false;
            resp.isChunked = false;
        }
        ResponseBuilder& withStatusCode(int code) {
            resp.statusCode = code;
            return *this;
        }
        ResponseBuilder& withStatusMessage(const std::string& message) {
            resp.statusMessage = message;
            return *this;
        }
        ResponseBuilder& withContentType(const std::string& type) {
            resp.contentType = type;
            return *this;
        }
        ResponseBuilder& withContentLength(int contentLength) {
            resp.contentLength = contentLength;
            return *this;
        }
        HttpResponse build() const { return resp; }
};

void assertEqualHttpResponse(const HttpResponse& want, const HttpResponse& got) {
    EXPECT_EQ(want.version, got.version);
    EXPECT_EQ(want.statusCode, got.statusCode);
    EXPECT_EQ(want.statusMessage, got.statusMessage);
    EXPECT_EQ(want.contentType, got.contentType);
    EXPECT_EQ(want.contentLanguage, got.contentLanguage);
    EXPECT_EQ(want.contentLength, got.contentLength);
    char gotBuffer[2048];
    std::string gotString;
    while (!got.body->isDone() || gotString.size() < (size_t)got.contentLength) {
        size_t bytesWritten = got.body->read(gotBuffer, 2048);
        gotString += std::string(gotBuffer, bytesWritten);
    }
    // std::cout << gotString << std::endl;
    EXPECT_EQ(want.contentLength, gotString.length());
}

TEST_P(TestGetHandler, ResponseFilling) {
    auto& params = GetParam();
    GetHandler handler;
    Connection* conn = new Connection({}, -1, NULL);
    conn->setState(Connection::Handling);
    handler.handle(conn, params.req, params.cfg);
    assertEqualHttpResponse(params.wantResp, conn->_response);
    delete conn;
}

INSTANTIATE_TEST_SUITE_P(
    GetHandlerTests, TestGetHandler,
    ::testing::Values(
        TestGetHandlerParams{                                                                       // 0
            RequestBuilder().withHeader("content-length", "48").build(),
            RouteConfigBuilder()
                .withErrorPage({{400, "/error_pages/400.html"}, {404, "/error_pages/404.html"}})
                .build(),
            ResponseBuilder()
                .withStatusCode(400)
                .withStatusMessage("Bad Request")
                .withContentType("text/html")
                .withContentLength(414)
                .build()
        },
        TestGetHandlerParams{                                                                       // 1
            RequestBuilder().withHeader("transfer-encoding", "chunked").build(),
            RouteConfigBuilder()
                .withErrorPage({{400, "/error_pages/400.html"}, {404, "/error_pages/404.html"}})
                .build(),
            ResponseBuilder()
                .withStatusCode(400)
                .withStatusMessage("Bad Request")
                .withContentType("text/html")
                .withContentLength(414)
                .build()
        },
        TestGetHandlerParams{                                                                       // 2
            RequestBuilder()
                .withUri("/Divine_Comedy.txt")
                .build(),
            RouteConfigBuilder().build(),
            ResponseBuilder()
                .withStatusCode(200)
                .withStatusMessage("OK")
                .withContentType("text/plain")
                .withContentLength(444)
                .build()
        },
        TestGetHandlerParams{                                                                       // 3
            RequestBuilder()
                .withUri("/index.html")
                .build(),
            RouteConfigBuilder().build(),
            ResponseBuilder()
                .withStatusCode(200)
                .withStatusMessage("OK")
                .withContentType("text/html")
                .withContentLength(1429)
                .build()
        },
        TestGetHandlerParams{                                                                       // 4
            RequestBuilder()
                .withUri("/")
                .build(),
            RouteConfigBuilder()
                .withIndex({"index.html"})
                .withAutoIndex(true)
                .build(),
            ResponseBuilder()
                .withStatusCode(200)
                .withStatusMessage("OK")
                .withContentType("text/html")
                .withContentLength(1429)
                .build()
        },
        TestGetHandlerParams{                                                                       // 5
            RequestBuilder().withUri("/nonexistent.txt").build(),
            RouteConfigBuilder()
                .withErrorPage({{400, "/error_pages/400.html"}, {404, "/error_pages/404.html"}})
                .build(),
            ResponseBuilder()
                .withStatusCode(404)
                .withStatusMessage("Not Found")
                .withContentType("text/html")
                .withContentLength(435)
                .build()
        },
        TestGetHandlerParams{                                                                       // 6
            RequestBuilder()
                .withHeader("content-length", "100")
                .withHeader("cransfer-encoding", "chunked")
                .build(),
            RouteConfigBuilder()
                .withErrorPage({{400, "/error_pages/400.html"}, {404, "/error_pages/404.html"}})
                .build(),
            ResponseBuilder()
                .withStatusCode(400)
                .withStatusMessage("Bad Request")
                .withContentType("text/html")
                .withContentLength(414)
                .build()
        },
        TestGetHandlerParams{                                                                       // 7
            RequestBuilder()
                .withUri("/")
                .build(),
            RouteConfigBuilder()
                .withIndex({})
                .withAutoIndex(false)
                .build(),
            ResponseBuilder()
                .withStatusCode(403)
                .withStatusMessage("Forbidden")
                .withContentType("text/plain")
                .withContentLength(9)
                .build()
        },
        TestGetHandlerParams{                                                                       // 8
            RequestBuilder()
                .withUri("")
                .build(),
            RouteConfigBuilder()
                .withIndex({"index.html"})
                .build(),
            ResponseBuilder()
                .withStatusCode(400)
                .withStatusMessage("Bad Request")
                .withContentType("text/plain")
                .withContentLength(11)
                .build()
        },
        TestGetHandlerParams{                                                                       // 9
            RequestBuilder()
                .withUri("/../../../src/GetHandler.cpp")
                .build(),
            RouteConfigBuilder().build(),
            ResponseBuilder()
                .withStatusCode(404)
                .withStatusMessage("Not Found")
                .withContentType("text/plain")
                .withContentLength(9)
                .build()
        },
        TestGetHandlerParams{                                                                       // 10
            RequestBuilder()
                .withUri("/image.jpg")
                .build(),
            RouteConfigBuilder().build(),
            ResponseBuilder()
                .withStatusCode(200)
                .withStatusMessage("OK")
                .withContentType("image/jpeg")
                .withContentLength(65459)
                .build()
        }
        // todo: a request of a file in no permission folder
    )
);