#include "Connection.h"
#include "HttpResponse.h"
#include "UploadHandler.h"
#include "test_UploadHandler_utils.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <gtest/gtest.h>

TEST(UploadHdlrTest, changeFileExisting) {
    std::string filename = "existing.txt";
    std::ofstream file(ROOT + PREFIX + filename);
    ASSERT_TRUE(file.is_open());
    file << "some content";
    file.close();

    int contentLength = 100;
    std::string body = getRandomString(contentLength);
    Connection* conn = setupConnWithContentLength(filename, contentLength);
    conn->_readBuf.assign(body);

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

TEST(UploadHdlrTest, testTempFileExistFileNotExist) {
    std::string filename = "notExisting.txt";
    size_t pos = 0;
    size_t batchSize = 10;

    size_t contentLength = 100;
    std::string body = getRandomString(contentLength);
    Connection* conn = setupConnWithContentLength(filename, contentLength);

    IHandler* uploadHdlr = new UploadHandler();
    while (conn->uploadCtx.state != UploadContext::UploadFinished) {
        conn->_readBuf.assign(body.substr(pos, batchSize));
        uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 10000, false, {}});
        struct stat statStruct;
        bool fileExists = !stat((ROOT + PREFIX + filename).c_str(), &statStruct);
        bool tempExists = !stat((ROOT + PREFIX + filename + ".temp").c_str(), &statStruct);
        if (conn->uploadCtx.state != UploadContext::UploadFinished) {
            EXPECT_EQ(fileExists, false);
            EXPECT_EQ(tempExists, true);
        } else {
            EXPECT_EQ(fileExists, true);
            EXPECT_EQ(tempExists, false);
        }
        pos += batchSize;
    }
    delete uploadHdlr;

    HttpResponse resp = conn->_response;
    delete conn; // need to delete conn to close the file and write to disk
    std::string gotFile1 = getFileContents(ROOT + PREFIX + filename);
    EXPECT_EQ(body.length(), gotFile1.length());
    EXPECT_EQ(body, gotFile1);
    EXPECT_EQ(201, resp.statusCode);
    EXPECT_EQ("Created", resp.statusMessage);

    removeFile(ROOT + PREFIX + filename);
}

TEST(UploadHdlrTest, testTempFileExistFileExist) {
    std::string filename = "existing.txt";
    std::ofstream file(ROOT + PREFIX + filename);
    ASSERT_TRUE(file.is_open());
    file << "some content";
    file.close();

    size_t pos = 0;
    size_t batchSize = 10;

    size_t contentLength = 100;
    std::string body = getRandomString(contentLength);
    Connection* conn = setupConnWithContentLength(filename, contentLength);

    IHandler* uploadHdlr = new UploadHandler();
    while (conn->uploadCtx.state != UploadContext::UploadFinished) {
        conn->_readBuf.assign(body.substr(pos, batchSize));
        uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 10000, false, {}});
        struct stat statStruct;
        bool fileExists = !stat((ROOT + PREFIX + filename).c_str(), &statStruct);
        bool tempExists = !stat((ROOT + PREFIX + filename + ".temp").c_str(), &statStruct);
        if (conn->uploadCtx.state != UploadContext::UploadFinished) {
            std::string gotFile2 = getFileContents(ROOT + PREFIX + filename);
            EXPECT_EQ("some content", gotFile2);
            EXPECT_EQ(fileExists, true);
            EXPECT_EQ(tempExists, true);
        } else {
            EXPECT_EQ(fileExists, true);
            EXPECT_EQ(tempExists, false);
        }
        pos += batchSize;
    }
    delete uploadHdlr;

    HttpResponse resp = conn->_response;
    delete conn; // need to delete conn to close the file and write to disk
    std::string gotFile1 = getFileContents(ROOT + PREFIX + filename);
    EXPECT_EQ(body.length(), gotFile1.length());
    EXPECT_EQ(body, gotFile1);
    EXPECT_EQ(200, resp.statusCode);
    EXPECT_EQ("OK", resp.statusMessage);

    removeFile(ROOT + PREFIX + filename);
}
