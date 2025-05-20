#include "Connection.h"
#include "UploadHandler.h"
#include "test_UploadHandler_utils.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <gtest/gtest.h>

std::string numberToHexString(int number) {
    std::ostringstream oss;
    oss << std::hex << number; // Convert number to hexadecimal
    return oss.str();
}

// TEST(UploadHdlrTest, chunked) {
//     IHandler* uploadHdlr = new UploadHandler();
//     size_t bodyLength = 1000;
//     std::string filename = "1.txt";
//     std::string body = getRandomString(bodyLength);

//     Connection* conn = setupConnWithTransferEncoding(filename);

//     size_t pos = 0;
//     while (conn->uploadCtx.state != UploadContext::UploadFinished) {
//         std::string chunk;
//         if (pos >= bodyLength) {
//             chunk = "0\r\n\r\n";
//         } else {
//             int chunkSize = getRandomNumber();
//             std::string bodyChunk = body.substr(pos, chunkSize);
//             chunk = numberToHexString(bodyChunk.length()) + "\r\n" + bodyChunk + "\r\n";
//             pos += chunkSize;
//         }
//         conn->_readBuf.assign(chunk);
//         uploadHdlr->handle(conn, conn->_request, {ROOT, {}, {}, 10000, false, {}});
//         EXPECT_EQ(conn->_readBuf.size(), 0);
//     }

//     // EXPECT_FALSE(conn->uploadCtx.file->is_open());
//     // EXPECT_EQ(conn->getState(), Connection::SendResponse);
//     // delete uploadHdlr;

//     // HttpResponse resp = conn->_response;
//     // delete conn;
//     // std::string gotFile1 = getFileContents(ROOT + PREFIX + filename);
//     // EXPECT_EQ(body.length(), gotFile1.length());
//     // EXPECT_EQ(body, gotFile1);
//     // EXPECT_EQ(201, resp.statusCode);
//     // EXPECT_EQ("Created", resp.statusMessage);

//     // removeFile(ROOT + PREFIX + filename);
// }
