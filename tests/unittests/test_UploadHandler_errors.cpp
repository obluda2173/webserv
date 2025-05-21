#include "Connection.h"
#include "HttpResponse.h"
#include "UploadHandler.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <gtest/gtest.h>

struct UploadHandlerErrorTestParams {
    size_t contentLength;
    size_t clientMaxBody;
    int wantStatusCode;
    std::string wantStatusMsg;
};

class UploadHdlrErrorTest : public testing::TestWithParam< UploadHandlerErrorTestParams > {};

TEST(UploadHdlrErrorTest, NoUploadOnSameFile) {
    IHandler* uploadHdlr = new UploadHandler();
    std::string filename = "doubleUpload.txt";

    std::string body = getRandomString(1000);
    Connection* conn1 = setupConnWithContentLength(filename, body.length());
    Connection* conn2 = setupConnWithContentLength(filename, body.length());

    conn1->_tempBody = std::string(body.substr(0, 500));
    uploadHdlr->handle(conn1, conn1->_request, {ROOT, {}, {}, 10000, false, {}});

    conn2->_tempBody = std::string(body.substr(0, 500));
    uploadHdlr->handle(conn2, conn2->_request, {ROOT, {}, {}, 10000, false, {}});

    EXPECT_EQ(409, conn2->_response.statusCode);
    EXPECT_EQ("Conflict", conn2->_response.statusMessage);
    EXPECT_EQ(conn2->getState(), Connection::SendResponse);
    delete conn2;

    conn1->_tempBody = std::string(body.substr(500));
    conn1->_bodyFinished = true;
    uploadHdlr->handle(conn1, conn1->_request, {ROOT, {}, {}, 10000, false, {}});

    EXPECT_EQ(201, conn1->_response.statusCode);
    EXPECT_EQ("Created", conn1->_response.statusMessage);

    conn2 = setupConnWithContentLength(filename, body.length());
    conn2->_tempBody = body;
    conn2->_bodyFinished = true;
    uploadHdlr->handle(conn2, conn2->_request, {ROOT, {}, {}, 10000, false, {}});
    EXPECT_EQ(200, conn2->_response.statusCode);
    EXPECT_EQ("OK", conn2->_response.statusMessage);

    delete conn1;
    delete conn2;
    delete uploadHdlr;

    removeFile(ROOT + PREFIX + filename);
}

// TEST_P(UploadHdlrErrorTest, errorTesting) {
//     UploadHandlerErrorTestParams params = GetParam();
//     size_t contentLength = params.contentLength;
//     size_t clientMaxBody = params.clientMaxBody;
//     RouteConfig cfg;
//     Connection* conn = setupConnWithContentLength("1.txt", contentLength);
//     IHandler* uploadHdlr = new UploadHandler();
//     uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, clientMaxBody, false, {}});

//     EXPECT_EQ(NULL, conn->uploadCtx.file);
//     EXPECT_EQ(params.wantStatusCode, conn->_response.statusCode);
//     EXPECT_EQ(params.wantStatusMsg, conn->_response.statusMessage);

//     delete conn;
//     delete uploadHdlr;
// }

// INSTANTIATE_TEST_SUITE_P(errorTests, UploadHdlrErrorTest,
//                          testing::Values( // UploadHandlerErrorTestParams{300, 200, 413, "Content Too Large"},
//                              UploadHandlerErrorTestParams{0, 200, 400, "Bad Request"}));

// TEST(UploadHdlrErrorTest, missingHeaders) {
//     RouteConfig cfg;
//     Connection* conn = new Connection({}, -1, "", NULL, NULL);
//     IHandler* uploadHdlr = new UploadHandler();
//     uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 0, false, {}});

//     EXPECT_EQ(NULL, conn->uploadCtx.file);
//     EXPECT_EQ(400, conn->_response.statusCode);
//     EXPECT_EQ("Bad Request", conn->_response.statusMessage);

//     delete conn;
//     delete uploadHdlr;
// }

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
    conn->_tempBody = body;
    conn->_bodyFinished = true;

    IHandler* uploadHdlr = new UploadHandler();
    uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 10000, false, {}});

    HttpResponse resp = conn->_response;
    delete conn; // need to delete conn to close the file and write to disk
    EXPECT_EQ(params.wantStatusCode, resp.statusCode);
    EXPECT_EQ(params.wantStatusMessage, resp.statusMessage);

    delete uploadHdlr;
}

INSTANTIATE_TEST_SUITE_P(
    fileErrorTests, UploadHdlrFileErrorsTest,
    ::testing::Values(UploadHdlrFileErrorsTestParams{"dirCannotOpen/notExisting.txt", 403, "Forbidden"},
                      UploadHdlrFileErrorsTestParams{"existDir/fileCannotOpen.txt", 403, "Forbidden"},
                      UploadHdlrFileErrorsTestParams{"existing/notExisting.txt", 409, "Conflict"},
                      UploadHdlrFileErrorsTestParams{"directory", 409, "Conflict"},
                      UploadHdlrFileErrorsTestParams{"notexistdir/existing.txt", 404, "Not Found"},
                      UploadHdlrFileErrorsTestParams{getRandomString(10) + "/existing.txt", 404, "Not Found"}));
