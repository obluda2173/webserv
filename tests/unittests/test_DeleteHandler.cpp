#include "test_handlers_utils.h"

struct TestDeleteHandlerParams {
    std::string setupFile;
    enum SetupType { NONE, FILE_SETUP, DIR_SETUP, READONLY_FILE, NON_EMPTY_DIR } setupType;
    HttpRequest req;
    RouteConfig cfg;
    HttpResponse wantResp;
    bool shouldExistAfter;
};

class TestDeleteHandler : public ::testing::TestWithParam< TestDeleteHandlerParams > {
  protected:
    void SetUp() override {
        const auto& param = GetParam();
        if (!param.setupFile.empty()) {
            std::string dir = param.setupFile.substr(0, param.setupFile.find_last_of('/'));
            mkdir(dir.c_str(), 0755);
            switch (param.setupType) {
            case TestDeleteHandlerParams::FILE_SETUP:
                std::ofstream(param.setupFile.c_str()) << "content";
                break;
            case TestDeleteHandlerParams::DIR_SETUP:
                mkdir(param.setupFile.c_str(), 0755);
                break;
            case TestDeleteHandlerParams::READONLY_FILE:
                std::ofstream(param.setupFile.c_str()) << "content";
                chmod(param.setupFile.c_str(), 0444);
                break;
            case TestDeleteHandlerParams::NON_EMPTY_DIR:
                mkdir(param.setupFile.c_str(), 0755);
                std::ofstream((param.setupFile + "/file.txt").c_str()) << "content";
                break;
            default:
                break;
            }
        }
    }
    void TearDown() override {
        const auto& param = GetParam();
        if (param.setupType == TestDeleteHandlerParams::NONE)
            return;
        struct stat st;
        if (stat(param.setupFile.c_str(), &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                std::string cmd = "rm -rf " + param.setupFile;
                system(cmd.c_str());
            } else {
                chmod(param.setupFile.c_str(), 0777);
                unlink(param.setupFile.c_str());
            }
        }
        std::string dir = param.setupFile.substr(0, param.setupFile.find_last_of('/'));
        rmdir(dir.c_str());
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
        TestDeleteHandlerParams{"./tests/unittests/test_root/delete_root", TestDeleteHandlerParams::NON_EMPTY_DIR,
                                RequestBuilder().withMethod("DELETE").withUri("/file.txt").build(),
                                RouteConfigBuilder().withRoot("./tests/unittests/test_root/delete_root").build(),
                                ResponseBuilder().withStatusCode(204).withStatusMessage("No Content").build(), true},
        TestDeleteHandlerParams{"./tests/unittests/test_root/delete_root/dir", TestDeleteHandlerParams::DIR_SETUP,
                                RequestBuilder().withMethod("DELETE").withUri("/delete_root/dir").build(),
                                RouteConfigBuilder().withRoot("./tests/unittests/test_root").build(),
                                ResponseBuilder().withStatusCode(204).withStatusMessage("No Content").build(), false},
        TestDeleteHandlerParams{"", TestDeleteHandlerParams::NONE,
                                RequestBuilder().withMethod("DELETE").withUri("/non_existing_file.txt").build(),
                                RouteConfigBuilder().withRoot("./tests/unittests/test_root/delete_root").build(),
                                ResponseBuilder()
                                    .withStatusCode(404)
                                    .withStatusMessage("Not Found")
                                    .withContentType("text/plain")
                                    .withContentLength(9)
                                    .build(),
                                false},
        TestDeleteHandlerParams{"", TestDeleteHandlerParams::NONE,
                                RequestBuilder().withMethod("DELETE").withUri("/non_existing_file.txt").build(),
                                RouteConfigBuilder()
                                    .withRoot("./tests/unittests/test_root")
                                    .withErrorPage({{400, "/error_pages/400.html"},
                                                    {403, "/error_pages/403.html"},
                                                    {404, "/error_pages/404.html"}})
                                    .build(),
                                ResponseBuilder()
                                    .withStatusCode(404)
                                    .withStatusMessage("Not Found")
                                    .withContentType("text/html")
                                    .withContentLength(435)
                                    .build(),
                                false},
        TestDeleteHandlerParams{
            "", TestDeleteHandlerParams::NONE, RequestBuilder().withMethod("DELETE").withUri("/error_pages").build(),
            RouteConfigBuilder()
                .withRoot("./tests/unittests/test_root")
                .withErrorPage(
                    {{400, "/error_pages/400.html"}, {403, "/error_pages/403.html"}, {404, "/error_pages/404.html"}})
                .build(),
            ResponseBuilder()
                .withStatusCode(500)
                .withStatusMessage("Internal Server Error")
                .withContentType("text/plain")
                .withContentLength(21)
                .build(),
            false},
        TestDeleteHandlerParams{"./tests/unittests/test_root/non_empty_dir", TestDeleteHandlerParams::NON_EMPTY_DIR,
                                RequestBuilder().withMethod("DELETE").withUri("/non_empty_dir").build(),
                                RouteConfigBuilder().withRoot("./tests/unittests/test_root").build(),
                                ResponseBuilder()
                                    .withStatusCode(500)
                                    .withStatusMessage("Internal Server Error")
                                    .withContentType("text/plain")
                                    .withContentLength(21)
                                    .build(),
                                true},
        TestDeleteHandlerParams{"./tests/unittests/test_root/readonly.txt", TestDeleteHandlerParams::READONLY_FILE,
                                RequestBuilder().withMethod("DELETE").withUri("/readonly.txt").build(),
                                RouteConfigBuilder().withRoot("./tests/unittests/test_root").build(),
                                ResponseBuilder()
                                    .withStatusCode(403)
                                    .withStatusMessage("Forbidden: Access denied")
                                    .withContentType("text/plain")
                                    .withContentLength(24)
                                    .build(),
                                true},
        TestDeleteHandlerParams{"./tests/unittests/test_root/valid_file.txt", TestDeleteHandlerParams::FILE_SETUP,
                                RequestBuilder().withMethod("DELETE").withUri("/../../../etc/passwd").build(),
                                RouteConfigBuilder().withRoot("./tests/unittests/test_root").build(),
                                ResponseBuilder()
                                    .withStatusCode(404)
                                    .withStatusMessage("Not Found")
                                    .withContentType("text/plain")
                                    .withContentLength(9)
                                    .build(),
                                true},
        TestDeleteHandlerParams{"./tests/unittests/test_root/secure_file.txt", TestDeleteHandlerParams::READONLY_FILE,
                                RequestBuilder().withMethod("DELETE").withUri("/secure_file.txt").build(),
                                RouteConfigBuilder()
                                    .withRoot("./tests/unittests/test_root")
                                    .withErrorPage({{403, "/error_pages/403.html"}})
                                    .build(),
                                ResponseBuilder()
                                    .withStatusCode(403)
                                    .withStatusMessage("Forbidden: Access denied")
                                    .withContentType("text/html")
                                    .withContentLength(412)
                                    .build(),
                                true}));
