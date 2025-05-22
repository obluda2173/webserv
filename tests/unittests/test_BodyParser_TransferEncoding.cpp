#include "BodyParser.h"
#include "Connection.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include <ostream>
#include <string>

class TransferEncodingTest : public testing::TestWithParam< int > {};

TEST_P(TransferEncodingTest, transferEncodingOneChunk) {
    int bodyLength = GetParam();
    BodyParser* bodyPrsr = new BodyParser();
    std::string body = getRandomString(bodyLength);

    std::ostringstream oss;
    oss << std::hex << body.length() << "\r\n";
    oss << body << "\r\n";
    oss << "0\r\n\r\n";
    std::string chunkedBody = oss.str();

    Connection* conn = setupConnectionTransferEncoding();

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

INSTANTIATE_TEST_SUITE_P(BodyParserTests, TransferEncodingTest, ::testing::Values(10, 1),
                         [](const testing::TestParamInfo< TransferEncodingTest::ParamType >& info) {
                             return std::to_string(info.param);
                         });

TEST_F(TransferEncodingTest, transferEncodingChunkInBatches) {
    BodyParser* bodyPrsr = new BodyParser();
    Connection* conn = setupConnectionTransferEncoding();

    size_t chunkSize = 98765;
    std::string chunk = getRandomString(chunkSize);
    std::string hex = "181CD";

    // assign only substring of hexNumber
    conn->_readBuf.assign("18");
    bodyPrsr->parse(conn);
    EXPECT_EQ(conn->_readBuf.size(), 0);
    EXPECT_FALSE(conn->_bodyFinished);

    conn->_readBuf.assign("1CD\r\n");
    bodyPrsr->parse(conn);
    EXPECT_EQ(conn->_readBuf.size(), 0);
    EXPECT_FALSE(conn->_bodyFinished);

    size_t batchSize = 1024;
    size_t pos = 0;
    while (pos < chunkSize) {
        std::string batch = chunk.substr(pos, batchSize);
        conn->_readBuf.assign(batch);
        bodyPrsr->parse(conn);
        EXPECT_EQ(conn->_readBuf.size(), 0);
        EXPECT_EQ(conn->_tempBody, batch);
        EXPECT_FALSE(conn->_bodyFinished);
        pos += batchSize;
    }
    conn->_readBuf.assign("\r\n");
    bodyPrsr->parse(conn);
    EXPECT_EQ(conn->_tempBody, "");
    EXPECT_FALSE(conn->_bodyFinished);

    conn->_readBuf.assign("0\r\n\r\nNextRequest");
    bodyPrsr->parse(conn);
    EXPECT_EQ(std::string(conn->_readBuf.data(), conn->_readBuf.size()), "NextRequest");
    EXPECT_EQ(conn->_tempBody, "");
    EXPECT_TRUE(conn->_bodyFinished);

    delete conn;
    delete bodyPrsr;
}

TEST_F(TransferEncodingTest, transferEncodingChunkInBatches2) {
    BodyParser* bodyPrsr = new BodyParser();
    Connection* conn = setupConnectionTransferEncoding();

    size_t chunkSize = 26;
    std::string chunk = getRandomString(chunkSize);
    std::string hex = "1A";

    // assign only substring of hexNumber
    conn->_readBuf.assign("1A");
    bodyPrsr->parse(conn);
    EXPECT_EQ(conn->_readBuf.size(), 0);
    EXPECT_EQ(conn->_tempBody, "");
    EXPECT_FALSE(conn->_bodyFinished);

    conn->_readBuf.assign("\r");
    bodyPrsr->parse(conn);
    EXPECT_EQ(conn->_readBuf.size(), 0);
    EXPECT_EQ(conn->_tempBody, "");
    EXPECT_FALSE(conn->_bodyFinished);

    conn->_readBuf.assign("\n");
    bodyPrsr->parse(conn);
    EXPECT_EQ(conn->_readBuf.size(), 0);
    EXPECT_EQ(conn->_tempBody, "");
    EXPECT_FALSE(conn->_bodyFinished);

    conn->_readBuf.assign(chunk);
    bodyPrsr->parse(conn);
    EXPECT_EQ(conn->_readBuf.size(), 0);
    EXPECT_EQ(conn->_tempBody, chunk);
    EXPECT_FALSE(conn->_bodyFinished);

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
