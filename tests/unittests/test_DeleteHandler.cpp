#include "test_handlers_utils.h"

struct TestDeleteHandlerParams {
    std::string setupFile;
    bool setupDir;
    HttpRequest req;
    RouteConfig cfg;
    HttpResponse wantResp;
    bool shouldExistAfter;
};

class TestDeleteHandler : public ::testing::TestWithParam<TestDeleteHandlerParams> {
protected:
    void SetUp() override {
        const auto& param = GetParam();
        if (!param.setupFile.empty()) {
            std::string dir = param.setupFile.substr(0, param.setupFile.find_last_of('/'));
            mkdir(dir.c_str(), 0755);
            if (param.setupDir) {
                mkdir(param.setupFile.c_str(), 0755);
            } else {
                std::ofstream(param.setupFile.c_str()) << "test content";
            }
        }
    }
    void TearDown() override {
        const auto& param = GetParam();
        if (!param.setupFile.empty()) {
            unlink(param.setupFile.c_str());
            std::string dir = param.setupFile.substr(0, param.setupFile.find_last_of('/'));
            rmdir(dir.c_str());
        }
    }
};

TEST_P(TestDeleteHandler, FileDeletionTests) {
    auto& param = GetParam();
    DeleteHandler handler;
    Connection* conn = new Connection({}, -1, nullptr);
    struct stat st;
    handler.handle(conn, param.req, param.cfg);
    assertEqualHttpResponse(param.wantResp, conn->_response);
    bool existsAfter = stat(param.setupFile.c_str(), &st) == 0;
    EXPECT_EQ(existsAfter, param.shouldExistAfter);
    delete conn;
}

INSTANTIATE_TEST_SUITE_P(
    DeleteHandlerTests, TestDeleteHandler,
    ::testing::Values(
        TestDeleteHandlerParams{
            "./tests/unittests/test_root/delete_root/file.txt",
            false,
            RequestBuilder()
                .withMethod("DELETE")
                .withUri("/file.txt")
                .build(),
            RouteConfigBuilder()
                .withRoot("./tests/unittests/test_root/delete_root")
                .build(),
            ResponseBuilder()
                .withStatusCode(204)
                .withStatusMessage("No Content")
                .build(),
            false
        },
        TestDeleteHandlerParams{
            "./tests/unittests/test_root/delete_root/empty_dir",
            true,
            RequestBuilder()
                .withMethod("DELETE")
                .withUri("/empty_dir")
                .build(),
            RouteConfigBuilder()
                .withRoot("./tests/unittests/test_root/delete_root")
                .build(),
            ResponseBuilder()
                .withStatusCode(204)
                .withStatusMessage("No Content")
                .build(),
            false
        },
        TestDeleteHandlerParams{
            "",
            false,
            RequestBuilder()
                .withMethod("DELETE")
                .withUri("/non_existing_file.txt")
                .build(),
            RouteConfigBuilder()
                .withRoot("./tests/unittests/test_root/delete_root")
                .build(),
            ResponseBuilder()
                .withStatusCode(404)
                .withStatusMessage("Not Found")
                .withContentType("text/plain")
                .withContentLength(9)
                .build(),
            false
        },
        TestDeleteHandlerParams{
            "",
            false,
            RequestBuilder()
                .withMethod("DELETE")
                .withUri("/non_existing_file.txt")
                .build(),
            RouteConfigBuilder()
                .withRoot("./tests/unittests/test_root")
                .withErrorPage({{400, "/error_pages/400.html"}, {403, "/error_pages/403.html"}, {404, "/error_pages/404.html"}})
                .build(),
            ResponseBuilder()
                .withStatusCode(404)
                .withStatusMessage("Not Found")
                .withContentType("text/html")
                .withContentLength(435)
                .build(),
            false
        }
    )
);