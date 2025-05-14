#include "Connection.h"
#include "HttpResponse.h"
#include "UploadHandler.h"
#include "test_Uploadhandler_utils.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <fstream>
#include <gtest/gtest.h>

// TODO: Make a test where the content of an existing file is changed
TEST(UploadHdlrTest, changeFileExisting) {
    std::string filename = "existing.txt";
    std::ofstream file(ROOT + PREFIX + filename);
    ASSERT_TRUE(file.is_open());
    file << "some content";
    file.close();

    int contentLength = 100;
    std::string body = getRandomString(contentLength);
    Connection* conn = setupConnWithContentLength(filename, contentLength);
    conn->setReadBuf(body);

    IHandler* uploadHdlr = new UploadHandler();
    uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 10000, false, {}});

    HttpResponse resp = conn->_response;
    delete conn; // need to delete conn to close the file and write to disk
    std::string gotFile1 = getFileContents(ROOT + PREFIX + filename);
    EXPECT_EQ(body.length(), gotFile1.length());
    EXPECT_EQ(body, gotFile1);
    EXPECT_EQ(200, resp.statusCode);
    EXPECT_EQ("OK", resp.statusMessage);

    removeFile(ROOT + PREFIX + filename);
    delete uploadHdlr;
}

TEST(UploadHdlrTest, filePathNotExist) {
    std::string filename = "existing.txt";
    std::string addPath = "notexistdir/";
    int contentLength = 100;
    std::string body = getRandomString(contentLength);
    Connection* conn = setupConnWithContentLength(addPath + filename, contentLength);
    conn->setReadBuf(body);

    IHandler* uploadHdlr = new UploadHandler();
    uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 10000, false, {}});

    HttpResponse resp = conn->_response;
    delete conn; // need to delete conn to close the file and write to disk
    EXPECT_EQ(400, resp.statusCode);
    EXPECT_EQ("Bad Request", resp.statusMessage);

    removeFile(ROOT + PREFIX + addPath + filename);
    delete uploadHdlr;
}

struct UploadHandlerTestParams {
    std::vector< std::string > filenames;
    std::vector< size_t > readBufsLengths;
    std::vector< size_t > bodyLengths;
    size_t batchSize; // amount of bites calling the upload handler handler function
    size_t clientMaxBody;
};

class UploadHdlrTest : public testing::TestWithParam< UploadHandlerTestParams > {
  protected:
    std::vector< std::string > filenames;
    std::vector< size_t > readBufsLengths;
    std::vector< size_t > contentLengths;
    std::vector< std::string > readBufs;
    std::vector< std::string > bodies;
    std::vector< Connection* > conns;
    size_t batchSize;
    size_t clientMaxBody;

  public:
    virtual void SetUp() override {
        UploadHandlerTestParams params = GetParam();
        batchSize = params.batchSize;
        clientMaxBody = params.clientMaxBody;

        filenames = params.filenames;
        readBufsLengths = params.readBufsLengths;
        contentLengths = params.bodyLengths;
        for (size_t i = 0; i < params.readBufsLengths.size(); i++) {
            readBufs.push_back(getRandomString(params.readBufsLengths[i]));
            bodies.push_back(readBufs[i].substr(0, params.bodyLengths[i]));
            conns.push_back(setupConnWithContentLength(params.filenames[i], bodies[i].length()));
        }
    }
};

TEST_P(UploadHdlrTest, concurrentUploadsParam) {
    IHandler* uploadHdlr = new UploadHandler();
    size_t pos = 0;
    size_t maxSize = *std::max_element(readBufsLengths.begin(), readBufsLengths.end());
    while (pos < maxSize) {
        for (size_t i = 0; i < filenames.size(); i++) {
            if (pos < readBufsLengths[i]) {
                conns[i]->setReadBuf(readBufs[i].substr(pos, batchSize));
                uploadHdlr->handle(conns[i], conns[i]->_request, {ROOT, {}, {}, clientMaxBody, false, {}});
            }
        }
        pos += batchSize;
    }
    delete uploadHdlr;

    for (size_t i = 0; i < filenames.size(); i++) {
        // the Connection holds a ofstream, which may not be flushed until deleting the Connection (which closes it)
        // therefore the connection needs to be closed before investigating the file
        HttpResponse resp = conns[i]->_response;
        delete conns[i];
        std::string gotFile1 = getFileContents(ROOT + PREFIX + filenames[i]);
        EXPECT_EQ(bodies[i].length(), gotFile1.length());
        EXPECT_EQ(bodies[i], gotFile1);
        EXPECT_EQ(201, resp.statusCode);
        EXPECT_EQ("Created", resp.statusMessage);

        removeFile(ROOT + PREFIX + filenames[i]);
    }
}

INSTANTIATE_TEST_SUITE_P(first, UploadHdlrTest,
                         testing::Values(UploadHandlerTestParams{{"1.txt"}, {1000}, {1000}, 1000, 1000},
                                         UploadHandlerTestParams{{"1.txt"}, {1000}, {300}, 10, 20000},
                                         UploadHandlerTestParams{{"1.txt"}, {1000}, {600}, 1000, 20000},
                                         UploadHandlerTestParams{{"1.txt"}, {1000}, {1000}, 10, 20000},
                                         UploadHandlerTestParams{{"1.txt"}, {1000}, {1000}, 1000, 20000},
                                         UploadHandlerTestParams{
                                             {"1.txt", "2.txt"}, {1000, 1000}, {444, 555}, 10, 20000}));

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

TEST(UploadHdlrTest, filePathNotSoGood) {       // it seems that this is a green test, which means the system deal with /./ automatically
    std::string filename = "/./existing.txt";

    int contentLength = 100;
    std::string body = getRandomString(contentLength);
    Connection* conn = setupConnWithContentLength(filename, contentLength);
    conn->setReadBuf(body);

    IHandler* uploadHdlr = new UploadHandler();
    uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 10000, false, {}});

    HttpResponse resp = conn->_response;
    delete conn; // need to delete conn to close the file and write to disk
    std::string gotFile1 = getFileContents(ROOT + PREFIX + filename);
    EXPECT_EQ(body.length(), gotFile1.length());
    EXPECT_EQ(body, gotFile1);
    EXPECT_EQ(201, resp.statusCode);
    EXPECT_EQ("Created", resp.statusMessage);

    removeFile(ROOT + PREFIX + filename);
    delete uploadHdlr;
}

TEST(UploadHdlrTest, filePathNotSoGoodAgain) {       // it seems that this is a green test, which means the system deal with /// automatically
    std::string filename = "//existing.txt";

    int contentLength = 100;
    std::string body = getRandomString(contentLength);
    Connection* conn = setupConnWithContentLength(filename, contentLength);
    conn->setReadBuf(body);

    IHandler* uploadHdlr = new UploadHandler();
    uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 10000, false, {}});

    HttpResponse resp = conn->_response;
    delete conn; // need to delete conn to close the file and write to disk
    std::string gotFile1 = getFileContents(ROOT + PREFIX + filename);
    EXPECT_EQ(body.length(), gotFile1.length());
    EXPECT_EQ(body, gotFile1);
    EXPECT_EQ(201, resp.statusCode);
    EXPECT_EQ("Created", resp.statusMessage);

    removeFile(ROOT + PREFIX + filename);
    delete uploadHdlr;
}

TEST(UploadHdlrTest, filePathNotSoGoodAgainAgain) {       // it seems that this is a green test, which means the system deal with // automatically
    std::string filename = "/existing.txt";

    int contentLength = 100;
    std::string body = getRandomString(contentLength);
    Connection* conn = setupConnWithContentLength(filename, contentLength);
    conn->setReadBuf(body);

    IHandler* uploadHdlr = new UploadHandler();
    uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 10000, false, {}});

    HttpResponse resp = conn->_response;
    delete conn; // need to delete conn to close the file and write to disk
    std::string gotFile1 = getFileContents(ROOT + PREFIX + filename);
    EXPECT_EQ(body.length(), gotFile1.length());
    EXPECT_EQ(body, gotFile1);
    EXPECT_EQ(201, resp.statusCode);
    EXPECT_EQ("Created", resp.statusMessage);

    removeFile(ROOT + PREFIX + filename);
    delete uploadHdlr;
}

TEST(UploadHdlrTest, filePathIsReallyBad) {
    std::string filename = "../existing.txt";

    int contentLength = 100;
    std::string body = getRandomString(contentLength);
    Connection* conn = setupConnWithContentLength(filename, contentLength);
    conn->setReadBuf(body);

    IHandler* uploadHdlr = new UploadHandler();
    uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 10000, false, {}});

    HttpResponse resp = conn->_response;
    delete conn; // need to delete conn to close the file and write to disk
    std::string gotFile1 = getFileContents(ROOT + PREFIX + filename);
    EXPECT_EQ(400, resp.statusCode);
    EXPECT_EQ("Bad Request", resp.statusMessage);

    removeFile(ROOT + PREFIX + filename);
    delete uploadHdlr;
}