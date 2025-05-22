#include "BodyParser.h"
#include "Connection.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include <ostream>
#include <string>

class TransferEncodingErrorTest : public ::testing::Test {
  protected:
    BodyParser* bodyPrsr;
    Connection* conn;

    void SetUp() override {
        bodyPrsr = new BodyParser();
        conn = setupConnectionTransferEncoding();
    }

    void TearDown() override {
        delete conn;
        delete bodyPrsr;
    }

    void testChunkedBodyError(const std::string& hexValue, std::string body, int expectedStatusCode,
                              const std::string& expectedStatusMessage) {
        std::ostringstream oss;
        oss << hexValue << "\r\n";
        oss << body << "\r\n";
        oss << "0\r\n\r\n";
        std::string chunkedBody = oss.str();

        conn->_readBuf.assign(chunkedBody);
        bodyPrsr->parse(conn);

        EXPECT_TRUE(conn->_bodyFinished);
        EXPECT_EQ(conn->getState(), Connection::SendResponse);
        EXPECT_EQ(conn->_response.statusCode, expectedStatusCode);
        EXPECT_EQ(conn->_response.statusMessage, expectedStatusMessage);
    }
};

TEST_F(TransferEncodingErrorTest, ChunkLongerThenChunkSize) {
    testChunkedBodyError("B", getRandomString(10), 400, "Bad Request");
}

TEST_F(TransferEncodingErrorTest, ContentTooLarge) {
    testChunkedBodyError("F000000000000000", getRandomString(10), 413, "Content Too Large");
}

TEST_F(TransferEncodingErrorTest, InvalidHexValue) {
    testChunkedBodyError("notAhex", getRandomString(10), 400, "Bad Request");
}
