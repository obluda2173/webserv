#include "Connection.h"
#include "HttpResponse.h"
#include "UploadHandler.h"
#include "test_UploadHandler_fixtures.h"
#include "test_UploadHandler_utils.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <gtest/gtest.h>

TEST(UploadHdlrTest2, uploading) {
    IHandler* uploadHdlr = new UploadHandler();
    int contentLength = 100;
    std::string filename = "1.txt";
    std::string body = getRandomString(contentLength);
    Connection* conn = setupConnWithContentLength(filename, contentLength);
    size_t pos = 0;
    size_t batchSize = 10;
    while (conn->uploadCtx.state != UploadContext::UploadFinished) {
        conn->_readBuf.assign(body.substr(pos, batchSize));
        uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 10000, false, {}});
        EXPECT_EQ(conn->_readBuf.size(), 0);
        pos += batchSize;
    }
    EXPECT_EQ(conn->getState(), Connection::SendResponse);
    delete uploadHdlr;

    HttpResponse resp = conn->_response;
    delete conn;
    std::string gotFile1 = getFileContents(ROOT + PREFIX + filename);
    EXPECT_EQ(body.length(), gotFile1.length());
    EXPECT_EQ(body, gotFile1);
    EXPECT_EQ(201, resp.statusCode);
    EXPECT_EQ("Created", resp.statusMessage);

    removeFile(ROOT + PREFIX + filename);
}

// these were the first tests
// The way that the UploadHandler is used in these tests is not "correct".
// Still they make sure that the right thing is saved into the files, especially it doesn't write more into than it
// should. The UploadHdlrTest2 are the tests representing the Usage of the UploadHdlrTest. I.e. they are testing that
// the _readBuf is set
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
                         testing::Values(UploadHandlerTestParams{{"example.txt/1.txt"}, {1000}, {1000}, 1000, 1000},
                                         UploadHandlerTestParams{{"1.txt"}, {1000}, {1000}, 1000, 1000},
                                         UploadHandlerTestParams{{"1.txt"}, {1000}, {300}, 10, 20000},
                                         UploadHandlerTestParams{{"1.txt"}, {1000}, {600}, 1000, 20000},
                                         UploadHandlerTestParams{{"1.txt"}, {1000}, {1000}, 10, 20000},
                                         UploadHandlerTestParams{{"1.txt"}, {1000}, {1000}, 1000, 20000},
                                         UploadHandlerTestParams{
                                             {"1.txt", "2.txt"}, {1000, 1000}, {444, 555}, 10, 20000}));
