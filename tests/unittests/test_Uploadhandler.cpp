#include "Connection.h"
#include "UploadHandler.h"
#include "test_Uploadhandler_utils.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <filesystem>
#include <gtest/gtest.h>

struct UploadHandlerTestParams {
    std::string filename;
    std::string readBuf;
    size_t bodyLength;
    size_t batchSize;
    size_t clientMaxBody;
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
        uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, params.clientMaxBody, false});
        pos += params.batchSize;
    }
    delete uploadHdlr;
    // cleanup(conn, ROOT + PREFIX + params.filename);
    delete conn;

    std::string gotFile = getFileContents(ROOT + PREFIX + params.filename);
    EXPECT_EQ(body.length(), gotFile.length());
    EXPECT_EQ(body, gotFile);

    std::string filepath = ROOT + PREFIX + params.filename;
    std::remove(filepath.data());
    ASSERT_FALSE(std::filesystem::exists(filepath));
}

INSTANTIATE_TEST_SUITE_P(
    firstTests, UploadHandlerTest,
    testing::Values(UploadHandlerTestParams{"example.txt", getRandomString(1000), 300, 10, 20000},
                    UploadHandlerTestParams{"example.txt", getRandomString(1000), 600, 1000, 20000},
                    UploadHandlerTestParams{"example.txt", getRandomString(1000), 1000, 10, 20000},
                    UploadHandlerTestParams{"example.txt", getRandomString(1000), 1000, 1000, 20000}));

TEST(UploadHandlerTest, concurrentUploads) {
    size_t batchSize = 10;
    std::string filename1 = "example1.txt";
    std::string filename2 = "example2.txt";
    std::string readBuf1 = getRandomString(1000);
    std::string readBuf2 = getRandomString(1000);
    size_t bodyLength1 = 444;
    size_t bodyLength2 = 555;

    std::string body1 = readBuf1.substr(0, bodyLength1);
    std::string body2 = readBuf2.substr(0, bodyLength2);

    Connection* conn1 = setupConnWithContentLength(filename1, body1.length());
    Connection* conn2 = setupConnWithContentLength(filename2, body2.length());
    IHandler* uploadHdlr = new UploadHandler();
    size_t pos = 0;
    while (pos < readBuf1.size()) {
        conn1->setReadBuf(readBuf1.substr(pos, batchSize));
        uploadHdlr->handle(conn1, conn1->_request, {ROOT, {}, {}, 10000, false});

        conn2->setReadBuf(readBuf2.substr(pos, batchSize));
        uploadHdlr->handle(conn2, conn2->_request, {ROOT, {}, {}, 10000, false});

        pos += batchSize;
    }
    delete uploadHdlr;
    delete conn1;
    delete conn2;

    std::string gotFile1 = getFileContents(ROOT + PREFIX + filename1);
    EXPECT_EQ(body1.length(), gotFile1.length());
    EXPECT_EQ(body1, gotFile1);
    std::string filepath1 = ROOT + PREFIX + filename1;
    std::remove(filepath1.data());
    ASSERT_FALSE(std::filesystem::exists(filepath1));

    std::string gotFile2 = getFileContents(ROOT + PREFIX + filename2);
    EXPECT_EQ(body2.length(), gotFile2.length());
    EXPECT_EQ(body2, gotFile2);
    std::string filepath2 = ROOT + PREFIX + filename2;
    std::remove(filepath2.data());
    ASSERT_FALSE(std::filesystem::exists(filepath2));
}

struct UploadHandlerTestParamsVector {
    std::vector<std::string> filenames;
    std::vector<size_t> readBufsLengths;
    std::vector<size_t> bodyLengths;
    size_t batchSize;
    size_t clientMaxBody;
};

class UploadHandlerTestVector : public testing::TestWithParam<UploadHandlerTestParamsVector> {};

TEST_P(UploadHandlerTestVector, concurrentUploadsParam) {
    UploadHandlerTestParamsVector params = GetParam();
    size_t batchSize = params.batchSize;
    std::vector<std::string> filenames = params.filenames;
    std::vector<size_t> readBufsLengths = params.readBufsLengths;
    std::vector<size_t> bodyLengths = params.bodyLengths;

    std::vector<std::string> readBufs;
    std::vector<std::string> bodies;
    std::vector<Connection*> conns;
    for (size_t i = 0; i < readBufsLengths.size(); i++) {
        readBufs.push_back(getRandomString(readBufsLengths[i]));
        bodies.push_back(readBufs[i].substr(0, bodyLengths[i]));
        conns.push_back(setupConnWithContentLength(filenames[i], bodies[i].length()));
    }

    IHandler* uploadHdlr = new UploadHandler();
    size_t pos = 0;
    size_t maxSize = *std::max_element(readBufsLengths.begin(), readBufsLengths.end());
    while (pos < maxSize) {
        for (size_t i = 0; i < filenames.size(); i++) {
            if (pos < readBufsLengths[i]) {
                conns[i]->setReadBuf(readBufs[i].substr(pos, batchSize));
                uploadHdlr->handle(conns[i], conns[i]->_request, {ROOT, {}, {}, params.clientMaxBody, false});
            }
        }
        pos += batchSize;
    }
    delete uploadHdlr;

    for (size_t i = 0; i < filenames.size(); i++) {
        delete conns[i];
        std::string gotFile1 = getFileContents(ROOT + PREFIX + filenames[i]);
        EXPECT_EQ(bodies[i].length(), gotFile1.length());
        EXPECT_EQ(bodies[i], gotFile1);
        std::string filepath = ROOT + PREFIX + filenames[i];
        std::remove(filepath.data());
        ASSERT_FALSE(std::filesystem::exists(filepath));
    }
}

INSTANTIATE_TEST_SUITE_P(first, UploadHandlerTestVector,
                         testing::Values(UploadHandlerTestParamsVector{
                             {"1.txt", "2.txt"}, {1000, 1000}, {444, 555}, 10, 20000}));
