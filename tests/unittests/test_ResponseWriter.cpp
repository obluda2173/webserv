#include "ResponseWriter.h"
#include <gtest/gtest.h>

TEST(ResponseWriterTest, firstTest) {
    HttpResponse resp;
    resp.statusCode = 200;
    resp.statusMessage = "OK";
    resp.contentLength = 4;
    resp.body = new StringBodyProvider("pong");
    resp.version = "HTTP/1.1";
    IResponseWriter* wrtr = new ResponseWriter(resp);
    char buffer[64];
    int writtenBytes = wrtr->write(buffer, 64);
    std::string got = std::string(buffer, writtenBytes);
    std::string want = "HTTP/1.1 200 OK\r\n"
                       "Content-Length: 4\r\n"
                       "\r\n"
                       "pong";
    EXPECT_EQ(want.c_str(), got.c_str());

    delete resp.body;
    delete wrtr;
}
