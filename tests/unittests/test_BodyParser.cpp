#include "BodyParser.h"
#include "Connection.h"
#include "test_main.h"
#include <gtest/gtest.h>
#include <string>

TEST(BodyParserTest, firstTest) {
    BodyParser* bodyPrsr = new BodyParser();
    int contentLength = 543;
    std::string body = getRandomString(contentLength);

    Connection* conn = new Connection({}, -1, 0, NULL, NULL);

    conn->_request.method = "";
    conn->_request.uri = "";
    conn->_request.version = "";
    conn->_request.headers["content-length"] = std::to_string(contentLength);
    conn->setState(Connection::Handling);
    conn->_bodyFinished = false;

    conn->_readBuf.assign(body);
    bodyPrsr->parse(conn);

    EXPECT_TRUE(conn->_bodyFinished);
    EXPECT_EQ(conn->_tempBody, body);

    delete conn;
    delete bodyPrsr;
}
