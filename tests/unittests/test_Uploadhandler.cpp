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

Connection* setupConnWithContentLength(std::string prefix, std::string filename, size_t contentLength) {
    Connection* conn = new Connection({}, -1, NULL, NULL);
    conn->_request.method = "POST";
    conn->_request.uri = prefix + filename;
    conn->_request.version = "HTTP/1.1";
    conn->_request.headers["content-length"] = std::to_string(contentLength);
    return conn;
}

Connection* setupConnWithoutContentLength(std::string prefix, std::string filename) {
    Connection* conn = new Connection({}, -1, NULL, NULL);
    conn->_request.method = "POST";
    conn->_request.uri = prefix + filename;
    conn->_request.version = "HTTP/1.1";
    return conn;
}

static std::string ROOT = "tests/unittests/test_files/UploadHandler";

void cleanup(Connection* conn, IHandler* hdlr, std::string filepath) {
    delete conn;
    delete hdlr;
    std::remove(filepath.data());
    ASSERT_FALSE(std::filesystem::exists(filepath));
}

// TEST(TestUploadHandler, TwoConnectionsUploadInChunks) {
//     // setup request
//     std::string filename1 = "example1.txt";
//     std::string filename2 = "example2.txt";
//     std::string prefix = "/uploads/";
//     std::string body1 = getRandomString(1000);
//     std::string body2 = getRandomString(1000);

//     Connection* conn1 = setupConnWithContentLength(prefix, filename1, body1);
//     Connection* conn2 = setupConnWithContentLength(prefix, filename2, body2);
//     IHandler* uploadHdlr = new UploadHandler();
//     int pos = 0;
//     while (pos < 1000) {
//         std::size_t size = 10;
//         conn1->setReadBuf(body1.substr(pos, size));
//         conn2->setReadBuf(body2.substr(pos, size));
//         pos += size;
//         uploadHdlr->handle(conn1, conn1->_request, {ROOT, {}, {}, 10000, false});
//         uploadHdlr->handle(conn2, conn2->_request, {ROOT, {}, {}, 10000, false});
//     }

//     std::string gotFile1 = getFileContents(ROOT + prefix + filename1);
//     EXPECT_EQ(body1, gotFile1);
//     cleanup(conn1, uploadHdlr, ROOT + prefix + filename1);

//     std::string gotFile2 = getFileContents(ROOT + prefix + filename2);
//     EXPECT_EQ(body2, gotFile2);
//     cleanup(conn2, NULL, ROOT + prefix + filename2);
// }

TEST(TestUploadHandler, uploadInChunksMismatchContent) {
    // setup request
    std::string filename = "example3.txt";
    std::string prefix = "/uploads/";
    std::string readBuf = getRandomString(1000);
    std::string body = readBuf.substr(0, 600);

    Connection* conn = setupConnWithContentLength(prefix, filename, body.length());
    IHandler* uploadHdlr = new UploadHandler();
    int pos = 0;
    while (pos < 1000) {
        std::size_t size = 10;
        conn->setReadBuf(readBuf.substr(pos, size));
        pos += size;
        uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 10000, false});
    }

    std::string gotFile = getFileContents(ROOT + prefix + filename);
    EXPECT_EQ(body.length(), gotFile.length());
    EXPECT_EQ(body, gotFile);
    cleanup(conn, uploadHdlr, ROOT + prefix + filename);
}

TEST(TestUploadHandler, uploadInChunksExactlyTheContent) {
    // setup request
    std::string filename = "example3.txt";
    std::string prefix = "/uploads/";
    std::string body = getRandomString(1000);

    Connection* conn = setupConnWithContentLength(prefix, filename, body.length());
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

TEST(TestUploadHandler, sendMoreThanContent) {
    std::string filename = "example5.txt";
    std::string prefix = "/uploads/";
    std::string readBuf = getRandomString(1000);
    std::string body = readBuf.substr(0, 600);

    Connection* conn = setupConnWithContentLength(prefix, filename, body.length());
    conn->setReadBuf(readBuf);
    IHandler* uploadHdlr = new UploadHandler();
    uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 10000, false});

    std::string gotFile = getFileContents(ROOT + prefix + filename);
    EXPECT_EQ(body.length(), gotFile.length());
    EXPECT_EQ(body, gotFile);
    cleanup(conn, uploadHdlr, ROOT + prefix + filename);
}

TEST(TestUploadHandler, firstTest) {
    // setup request
    std::string filename = "example.txt";
    std::string prefix = "/uploads/";
    std::string body = getRandomString(1000);

    Connection* conn = setupConnWithContentLength(prefix, filename, body.length());
    conn->setReadBuf(body);
    IHandler* uploadHdlr = new UploadHandler();
    uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 10000, false});

    std::string gotFile = getFileContents(ROOT + prefix + filename);
    EXPECT_EQ(body.length(), gotFile.length());
    EXPECT_EQ(body, gotFile);
    cleanup(conn, uploadHdlr, ROOT + prefix + filename);

    filename = "example2.txt";
    prefix = "/uploads/";
    body = getRandomString(1000);

    conn = setupConnWithContentLength(prefix, filename, body.length());
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
