#include "Connection.h"
#include "test_handlers_utils.h"

struct TestGetHandlerParams {
    HttpRequest req;
    RouteConfig cfg;
    HttpResponse wantResp;
};

class TestGetHandler : public ::testing::TestWithParam< TestGetHandlerParams > {};

TEST_P(TestGetHandler, ResponseFilling) {
    auto& params = GetParam();
    GetHandler handler;
    Connection* conn = new Connection({}, -1, "", nullptr);
    conn->setState(Connection::Handling);
    handler.handle(conn, params.req, params.cfg);
    assertEqualHttpResponse(params.wantResp, conn->_response); // Use shared function
    delete conn;
}

INSTANTIATE_TEST_SUITE_P(
    GetHandlerTests, TestGetHandler,
    ::testing::Values(
        TestGetHandlerParams{// 0 invalid body header
                             RequestBuilder().withMethod("GET").withHeader("content-length", "48").build(),
                             RouteConfigBuilder()
                                 .withErrorPage({{400, "/error_pages/400.html"}, {404, "/error_pages/404.html"}})
                                 .build(),
                             ResponseBuilder()
                                 .withStatusCode(400)
                                 .withStatusMessage("Bad Request")
                                 .withContentType("text/html")
                                 .withContentLength(414)
                                 .build()},
        TestGetHandlerParams{// 1 invalid body header
                             RequestBuilder().withMethod("GET").withHeader("transfer-encoding", "chunked").build(),
                             RouteConfigBuilder()
                                 .withErrorPage({{400, "/error_pages/400.html"}, {404, "/error_pages/404.html"}})
                                 .build(),
                             ResponseBuilder()
                                 .withStatusCode(400)
                                 .withStatusMessage("Bad Request")
                                 .withContentType("text/html")
                                 .withContentLength(414)
                                 .build()},
        TestGetHandlerParams{// 2 valid request of .txt
                             RequestBuilder().withMethod("GET").withUri("/Divine_Comedy.txt").build(),
                             RouteConfigBuilder().build(),
                             ResponseBuilder()
                                 .withStatusCode(200)
                                 .withStatusMessage("OK")
                                 .withContentType("text/plain")
                                 .withContentLength(444)
                                 .build()},
        TestGetHandlerParams{// 3 valid request of .html
                             RequestBuilder().withMethod("GET").withUri("/index.html").build(),
                             RouteConfigBuilder().build(),
                             ResponseBuilder()
                                 .withStatusCode(200)
                                 .withStatusMessage("OK")
                                 .withContentType("text/html")
                                 .withContentLength(1429)
                                 .build()},
        TestGetHandlerParams{// 4 autoindex check
                             RequestBuilder().withMethod("GET").withUri("/").build(),
                             RouteConfigBuilder().withIndex({"index.html"}).withAutoIndex(true).build(),
                             ResponseBuilder()
                                 .withStatusCode(200)
                                 .withStatusMessage("OK")
                                 .withContentType("text/html")
                                 .withContentLength(1429)
                                 .build()},
        TestGetHandlerParams{// 5 non existing file
                             RequestBuilder().withMethod("GET").withUri("/nonexistent.txt").build(),
                             RouteConfigBuilder()
                                 .withErrorPage({{400, "/error_pages/400.html"}, {404, "/error_pages/404.html"}})
                                 .build(),
                             ResponseBuilder()
                                 .withStatusCode(404)
                                 .withStatusMessage("Not Found")
                                 .withContentType("text/html")
                                 .withContentLength(435)
                                 .build()},
        TestGetHandlerParams{// 6 invalid body header
                             RequestBuilder()
                                 .withMethod("GET")
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
                                 .build()},
        TestGetHandlerParams{// 7 no index, autoindex and uri
                             RequestBuilder().withMethod("GET").withUri("/").build(),
                             RouteConfigBuilder().withIndex({}).withAutoIndex(false).build(),
                             ResponseBuilder()
                                 .withStatusCode(403)
                                 .withStatusMessage("Forbidden")
                                 .withContentType("text/plain")
                                 .withContentLength(0)
                                 .build()},
        TestGetHandlerParams{// 8 with index but without autoindex
                             RequestBuilder().withMethod("GET").withUri("").build(),
                             RouteConfigBuilder().withIndex({"index.html"}).build(),
                             ResponseBuilder()
                                 .withStatusCode(400)
                                 .withStatusMessage("Bad Request")
                                 .withContentType("text/plain")
                                 .withContentLength(0)
                                 .build()},
        TestGetHandlerParams{// 10 image serving
                             RequestBuilder().withMethod("GET").withUri("/image.jpg").build(),
                             RouteConfigBuilder().build(),
                             ResponseBuilder()
                                 .withStatusCode(200)
                                 .withStatusMessage("OK")
                                 .withContentType("image/jpeg")
                                 .withContentLength(65459)
                                 .build()},
        TestGetHandlerParams{
            // 11 no permission folder
            RequestBuilder().withMethod("GET").withUri("/no_permission_folder/secret_file.txt").build(),
            RouteConfigBuilder()
                .withErrorPage(
                    {{400, "/error_pages/400.html"}, {403, "/error_pages/403.html"}, {404, "/error_pages/404.html"}})
                .build(),
            ResponseBuilder()
                .withStatusCode(404)
                .withStatusMessage("Not Found")
                .withContentType("text/html")
                .withContentLength(435)
                .build()},
        TestGetHandlerParams{// 12 test directory listing (content-length changes with changing test_root)
                             RequestBuilder().withMethod("GET").withUri("/").build(),
                             RouteConfigBuilder()
                                 .withErrorPage({{400, "/error_pages/400.html"},
                                                 {403, "/error_pages/403.html"},
                                                 {404, "/error_pages/404.html"}})
                                 .withAutoIndex(true)
                                 .build(),
                             ResponseBuilder()
                                 .withStatusCode(200)
                                 .withStatusMessage("OK")
                                 .withContentType("text/html")
                                 .withContentLength(460)
                                 .build()},
        TestGetHandlerParams{// 15 invalid percent encoding
                             RequestBuilder().withMethod("GET").withUri("/file%2with%20space.txt").build(),
                             RouteConfigBuilder().build(),
                             ResponseBuilder()
                                 .withStatusCode(404)
                                 .withStatusMessage("Not Found")
                                 .withContentType("text/plain")
                                 .withContentLength(0)
                                 .build()},
        TestGetHandlerParams{// 16 default error page (no custom config)
                             RequestBuilder().withMethod("GET").withUri("/ghost_file.txt").build(),
                             RouteConfigBuilder().build(),
                             ResponseBuilder()
                                 .withStatusCode(404)
                                 .withStatusMessage("Not Found")
                                 .withContentType("text/plain")
                                 .withContentLength(0)
                                 .build()},
        TestGetHandlerParams{// 17 unknown MIME type
                             RequestBuilder().withMethod("GET").withUri("/unknown.xyz").build(),
                             RouteConfigBuilder().build(),
                             ResponseBuilder()
                                 .withStatusCode(200)
                                 .withStatusMessage("OK")
                                 .withContentType("application/octet-stream")
                                 .withContentLength(13)
                                 .build()},
        TestGetHandlerParams{// 18 multiple .. in path
                             RequestBuilder().withMethod("GET").withUri("/../../../../etc/passwd").build(),
                             RouteConfigBuilder().build(),
                             ResponseBuilder()
                                 .withStatusCode(404)
                                 .withStatusMessage("Not Found")
                                 .withContentType("text/plain")
                                 .withContentLength(0)
                                 .build()}));
