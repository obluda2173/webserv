#include "BodyParser.h"
#include "Connection.h"
#include "test_main.h"
#include <gtest/gtest.h>
#include <ostream>
#include <string>

TEST(BodyParserTest, transferEncoding) {
    BodyParser* bodyPrsr = new BodyParser();
    int contentLength = 10;
    std::string body = getRandomString(10);

    std::ostringstream oss;
    oss << std::hex << body.length() << "\r\n";
    oss << body << "\r\n";
    oss << "0\r\n\r\n";
    std::string chunkedBody = oss.str();

    Connection* conn = new Connection({}, -1, "", NULL, NULL);
    conn->_request.headers["transfer-encoding"] = std::to_string(contentLength);
    conn->setState(Connection::Handling);
    conn->_bodyFinished = false;

    conn->_readBuf.assign(chunkedBody);
    bodyPrsr->parse(conn);

    std::string bodyReceived;
    bodyReceived += conn->_tempBody;
    EXPECT_EQ(conn->_readBuf.size(), 0);
    EXPECT_EQ(body, bodyReceived);
    EXPECT_TRUE(conn->_bodyFinished);

    delete conn;
    delete bodyPrsr;
}

// std::string createChunkedBody(const std::string& body) {
//     std::ostringstream oss;
//     std::size_t pos = 0;

//     while (pos < body.length()) {
//         std::size_t len = std::min((size_t)getRandomNumber(0, 50), body.length() - pos);
//         // Add chunk size in hexadecimal form
//         oss << std::hex << len << "\r\n";
//         // Add chunk data
//         oss << body.substr(pos, len) << "\r\n";
//         pos += len;
//     }
//     // Add the final zero-sized chunk to indicate the end
//     oss << "0\r\n\r\n";
//     return oss.str();
// }

// TEST(BodyParserTest, transferEncoding) {
//     BodyParser* bodyPrsr = new BodyParser();
//     int contentLength = 12345;
//     std::string body = getRandomString(contentLength);
//     std::string chunkedBody = createChunkedBody(getRandomString(contentLength));

//     Connection* conn = new Connection({}, -1, "", NULL, NULL);
//     conn->_request.headers["transfer-encoding"] = std::to_string(contentLength);
//     conn->setState(Connection::Handling);
//     conn->_bodyFinished = false;

//     size_t pos = 0;
//     std::string bodyReceived;
//     while (!conn->_bodyFinished) {
//         int batchSize = getRandomNumber(0, 50);

//         std::string bodyBatch = chunkedBody.substr(pos, batchSize);
//         pos += batchSize;

//         conn->_readBuf.assign(bodyBatch);
//         bodyPrsr->parse(conn);

//         bodyReceived += conn->_tempBody;
//         EXPECT_EQ(conn->_readBuf.size(), 0);
//     }

//     EXPECT_TRUE(conn->_bodyFinished);

//     delete conn;
//     delete bodyPrsr;
// }
