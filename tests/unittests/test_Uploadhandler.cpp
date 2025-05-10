#include "Connection.h"
#include "HttpRequest.h"
#include "UploadHandler.h"
#include "test_main.h"
#include <fstream>
#include <gtest/gtest.h>

std::string getFileContents(const std::string& filename) {
    std::ifstream file(filename);
    if (!file)
        throw std::ios_base::failure("Error opening file");

    std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    return contents;
}
TEST(TestUploadHandler, firstTest) {
    // setup request
    std::string root = "tests/unittests/test_files/UploadHandler";
    std::string filename = "example.txt";
    std::string body = getRandomString(1000);

    Connection* conn = new Connection({}, -1, NULL, NULL);
    conn->_request.method = "POST";
    conn->_request.uri = "/upload/" + filename;
    conn->_request.version = "HTTP/1.1";
    conn->_request.headers["content-length"] = body.length();
    conn->setReadBuf(body);

    IHandler* uploadHdlr = new UploadHandler();
    uploadHdlr->handle(conn, conn->_request, {root, {}, {}, 10000, false});

    std::string gotFile = getFileContents("tests/unittests/test_files/UploadHandler/example.txt");

    EXPECT_EQ(body, gotFile);
    // assert that example.txt was created and check that the file content is correct
    delete conn;
    delete uploadHdlr;
    std::remove((root + "/" + filename).data());
}
