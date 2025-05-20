#include "BodyParser.h"
#include "Connection.h"
#include "test_main.h"
#include <gtest/gtest.h>
#include <string>

TEST(BodyParserTest, firstTest) {
    BodyParser* bodyPrsr = new BodyParser();
    int contentLength = 12345;
    std::string body = getRandomString(contentLength);

    Connection* conn = new Connection({}, -1, 0, NULL, NULL);

    conn->_request.method = "";
    conn->_request.uri = "";
    conn->_request.version = "";
    conn->_request.headers["content-length"] = std::to_string(contentLength);
    conn->setState(Connection::Handling);
    conn->_bodyFinished = false;

    size_t pos = 0;
    while (!conn->_bodyFinished) {
        int chunkSize = getRandomNumber(10, 50);
        std::string bodyChunk = body.substr(pos, chunkSize);
        pos += chunkSize;

        conn->_readBuf.assign(bodyChunk);
        bodyPrsr->parse(conn);
        EXPECT_EQ(conn->_tempBody, bodyChunk);
    }

    EXPECT_TRUE(conn->_bodyFinished);

    delete conn;
    delete bodyPrsr;
}

TEST(BodyParserTestContentLength, concurrently) {
    BodyParser* bodyPrsr = new BodyParser();

    Connection* conn = new Connection({}, -1, 0, NULL, NULL);
    int contentLength = 9876;

    conn->_request.method = "";
    conn->_request.uri = "";
    conn->_request.version = "";
    conn->_request.headers["content-length"] = std::to_string(contentLength);
    conn->setState(Connection::Handling);
    conn->_bodyFinished = false;

    std::string body = getRandomString(contentLength);
    std::string bytes = body + getRandomString(10000 - contentLength);

    size_t pos = 0;
    while (pos < bytes.length()) {
        int chunkSize = getRandomNumber(10, 50);
        std::string bytesChunk = bytes.substr(pos, chunkSize);
        pos += chunkSize;
        conn->_readBuf.assign(bytesChunk);

        bodyPrsr->parse(conn);

        if (pos <= body.length()) {
            EXPECT_EQ(conn->_tempBody, bytesChunk);
        }
    }
}
