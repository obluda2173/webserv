#include "Connection.h"
#include "HttpRequest.h"
#include "UploadHandler.h"
#include "test_main.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

std::string getFileContents(const std::string& filename) {
    std::ifstream file(filename);
    if (!file)
        throw std::ios_base::failure("Error opening file");

    std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    return contents;
}

Connection* setupConnWithContentLength(std::string prefix, std::string filename, std::string body) {
    Connection* conn = new Connection({}, -1, NULL, NULL);
    conn->_request.method = "POST";
    conn->_request.uri = prefix + filename;
    conn->_request.version = "HTTP/1.1";
    conn->_request.headers["content-length"] = body.length();
    return conn;
}

static std::string ROOT = "tests/unittests/test_files/UploadHandler";

void cleanup(Connection* conn, IHandler* hdlr, std::string filepath) {
    delete conn;
    delete hdlr;
    std::remove(filepath.data());
    ASSERT_FALSE(std::filesystem::exists(filepath));
}

TEST(TestUploadHandler, inMultiParts) {
    // setup request
    std::string filename = "example3.txt";
    std::string prefix = "/uploads/";
    std::string body = getRandomString(1000);

    Connection* conn = setupConnWithContentLength(prefix, filename, body);
    IHandler* uploadHdlr = new UploadHandler();
    int pos = 0;
    while (pos < 1000) {
        std::size_t size = 10;
        conn->setReadBuf(body.substr(pos, size));
        pos += size;
        uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 10000, false});
    }

    std::string gotFile = getFileContents(ROOT + prefix + filename);
    EXPECT_EQ(body, gotFile);
    cleanup(conn, uploadHdlr, ROOT + prefix + filename);
}

TEST(TestUploadHandler, firstTest) {
    // setup request
    std::string filename = "example.txt";
    std::string prefix = "/uploads/";
    std::string body = getRandomString(1000);

    Connection* conn = setupConnWithContentLength(prefix, filename, body);
    conn->setReadBuf(body);
    IHandler* uploadHdlr = new UploadHandler();
    uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 10000, false});

    std::string gotFile = getFileContents(ROOT + prefix + filename);
    cleanup(conn, uploadHdlr, ROOT + prefix + filename);
    EXPECT_EQ(body, gotFile);

    filename = "example2.txt";
    prefix = "/uploads/";
    body = getRandomString(1000);

    conn = setupConnWithContentLength(prefix, filename, body);
    conn->setReadBuf(body);
    uploadHdlr = new UploadHandler();
    uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 10000, false});

    gotFile = getFileContents(ROOT + prefix + filename);

    EXPECT_EQ(body, gotFile);
    delete conn;
    delete uploadHdlr;
    std::remove((ROOT + prefix + filename).data());
    ASSERT_FALSE(std::filesystem::exists(ROOT + prefix + filename));
}
