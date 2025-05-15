#include "Connection.h"
#include "HttpResponse.h"
#include "UploadHandler.h"
#include "test_Uploadhandler_utils.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <gtest/gtest.h>

// TODO: Make a test where the content of an existing file is changed
struct UploadHandlerErrorTestParams {
    size_t contentLength;
    size_t clientMaxBody;
    int wantStatusCode;
    std::string wantStatusMsg;
};

class UploadHdlrErrorTest : public testing::TestWithParam< UploadHandlerErrorTestParams > {};

TEST_P(UploadHdlrErrorTest, errorTesting) {
    UploadHandlerErrorTestParams params = GetParam();
    size_t contentLength = params.contentLength;
    size_t clientMaxBody = params.clientMaxBody;
    RouteConfig cfg;
    Connection* conn = setupConnWithContentLength("1.txt", contentLength);
    IHandler* uploadHdlr = new UploadHandler();
    uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, clientMaxBody, false, {}});

    EXPECT_EQ(NULL, conn->uploadCtx.file);
    EXPECT_EQ(params.wantStatusCode, conn->_response.statusCode);
    EXPECT_EQ(params.wantStatusMsg, conn->_response.statusMessage);

    delete conn;
    delete uploadHdlr;
}

INSTANTIATE_TEST_SUITE_P(errorTests, UploadHdlrErrorTest,
                         testing::Values(UploadHandlerErrorTestParams{300, 200, 413, "Content Too Large"},
                                         UploadHandlerErrorTestParams{0, 200, 400, "Bad Request"}));

TEST(UploadHdlrErrorTest, missingHeaders) {
    RouteConfig cfg;
    Connection* conn = new Connection({}, -1, NULL, NULL);
    IHandler* uploadHdlr = new UploadHandler();
    uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 0, false, {}});

    EXPECT_EQ(NULL, conn->uploadCtx.file);
    EXPECT_EQ(400, conn->_response.statusCode);
    EXPECT_EQ("Bad Request", conn->_response.statusMessage);

    delete conn;
    delete uploadHdlr;
}

struct UploadHdlrFileErrorsTestParams {
    std::string path;
    int wantStatusCode;
    std::string wantStatusMessage;
};

class UploadHdlrFileErrorsTest : public testing::TestWithParam< UploadHdlrFileErrorsTestParams > {};

TEST_P(UploadHdlrFileErrorsTest, filePathNotExist) {
    UploadHdlrFileErrorsTestParams params = GetParam();
    int contentLength = 100;
    std::string body = getRandomString(contentLength);
    Connection* conn = setupConnWithContentLength(params.path, contentLength);
    conn->setReadBuf(body);

    IHandler* uploadHdlr = new UploadHandler();
    uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 10000, false, {}});

    HttpResponse resp = conn->_response;
    delete conn; // need to delete conn to close the file and write to disk
    EXPECT_EQ(params.wantStatusCode, resp.statusCode);
    EXPECT_EQ(params.wantStatusMessage, resp.statusMessage);

    delete uploadHdlr;
}

INSTANTIATE_TEST_SUITE_P(fileErrorTests, UploadHdlrFileErrorsTest,
                         ::testing::Values(UploadHdlrFileErrorsTestParams{"existing/notExisting.txt", 409, "Conflict"},
                                           UploadHdlrFileErrorsTestParams{"directory", 409, "Conflict"},
                                           UploadHdlrFileErrorsTestParams{"notexistdir/existing.txt", 404, "Not Found"},
                                           UploadHdlrFileErrorsTestParams{getRandomString(10) + "/existing.txt", 404,
                                                                          "Not Found"}));

// TEST(UploadHdlrTest,
//      filePathNotSoGood) { // it seems that this is a green test, which means the system deal with /./ automatically
//     std::string filename = "/./existing.txt"; // TODO: path not normalized (normalization before routing)

//     int contentLength = 100;
//     std::string body = getRandomString(contentLength);
//     Connection* conn = setupConnWithContentLength(filename, contentLength);
//     conn->setReadBuf(body);

//     IHandler* uploadHdlr = new UploadHandler();
//     uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 10000, false, {}});

//     HttpResponse resp = conn->_response;
//     delete conn; // need to delete conn to close the file and write to disk
//     std::string gotFile1 = getFileContents(ROOT + PREFIX + filename);
//     EXPECT_EQ(body.length(), gotFile1.length());
//     EXPECT_EQ(body, gotFile1);
//     EXPECT_EQ(201, resp.statusCode);
//     EXPECT_EQ("Created", resp.statusMessage);

//     removeFile(ROOT + PREFIX + filename);
//     delete uploadHdlr;
// }
