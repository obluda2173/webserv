#include "Connection.h"
#include "HttpResponse.h"
#include "UploadHandler.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <gtest/gtest.h>

TEST(UploadHdlrTest2, uploading) {
    IHandler* uploadHdlr = new UploadHandler();
    int contentLength = 100;
    std::string filename = "1.txt";
    std::string body = getRandomString(contentLength);
    Connection* conn = setupConnWithContentLength(filename, contentLength);
    size_t pos = 0;
    size_t batchSize = 10;
    conn->_bodyFinished = false;
    while (conn->getState() != Connection::SendResponse) {
        conn->_tempBody = body.substr(pos, batchSize);
        pos += batchSize;
        if (pos >= body.length())
            conn->_bodyFinished = true;

        uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 0, false, {}});
    }
    delete uploadHdlr;

    HttpResponse resp = conn->_response;
    std::string gotFile1 = getFileContents(ROOT + PREFIX + filename);
    EXPECT_EQ(body.length(), gotFile1.length());
    EXPECT_EQ(body, gotFile1);
    EXPECT_EQ(201, resp.statusCode);
    EXPECT_EQ("Created", resp.statusMessage);

    removeFile(ROOT + PREFIX + filename);
    delete conn;
}
