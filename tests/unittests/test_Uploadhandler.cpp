#include "Connection.h"
#include "UploadHandler.h"
#include "test_Uploadhandler_utils.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <gtest/gtest.h>

struct UploadHandlerTestParams {
    std::string filename;
    std::string readBuf;
    size_t bodyLength;
    size_t batchSize;
};

class UploadHandlerTest : public testing::TestWithParam<UploadHandlerTestParams> {};

TEST_P(UploadHandlerTest, firstTest) {
    UploadHandlerTestParams params = GetParam();
    std::string body = params.readBuf.substr(0, params.bodyLength);

    Connection* conn = setupConnWithContentLength(params.filename, body.length());
    IHandler* uploadHdlr = new UploadHandler();
    size_t pos = 0;
    while (pos < params.readBuf.size()) {
        conn->setReadBuf(params.readBuf.substr(pos, params.batchSize));
        uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 10000, false});
        pos += params.batchSize;
    }
    delete uploadHdlr;

    std::string gotFile = getFileContents(ROOT + PREFIX + params.filename);
    EXPECT_EQ(body.length(), gotFile.length());
    EXPECT_EQ(body, gotFile);
    cleanup(conn, ROOT + PREFIX + params.filename);
}

INSTANTIATE_TEST_SUITE_P(firstTests, UploadHandlerTest,
                         testing::Values(UploadHandlerTestParams{"example.txt", getRandomString(1000), 300, 10},
                                         UploadHandlerTestParams{"example.txt", getRandomString(1000), 600, 1000},
                                         UploadHandlerTestParams{"example.txt", getRandomString(1000), 1000, 10},
                                         UploadHandlerTestParams{"example.txt", getRandomString(1000), 1000, 1000}));
