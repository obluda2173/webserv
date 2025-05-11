#include "Connection.h"
#include "HttpRequest.h"
#include "UploadHandler.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

static std::string ROOT = "tests/unittests/test_files/UploadHandler";
static std::string PREFIX = "/uploads";

std::string getFileContents(const std::string& filename) {
    std::ifstream file(filename);
    if (!file)
        throw std::ios_base::failure("Error opening file");

    std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    return contents;
}

Connection* setupConnWithContentLength(std::string filename, size_t contentLength) {
    Connection* conn = new Connection({}, -1, NULL, NULL);
    conn->_request.method = "POST";
    conn->_request.uri = PREFIX + filename;
    conn->_request.version = "HTTP/1.1";
    conn->_request.headers["content-length"] = std::to_string(contentLength);
    return conn;
}

Connection* setupConnWithoutContentLength(std::string filename) {
    Connection* conn = new Connection({}, -1, NULL, NULL);
    conn->_request.method = "POST";
    conn->_request.uri = PREFIX + filename;
    conn->_request.version = "HTTP/1.1";
    return conn;
}

void cleanup(Connection* conn, std::string filepath) {
    delete conn;
    std::remove(filepath.data());
    ASSERT_FALSE(std::filesystem::exists(filepath));
}

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
