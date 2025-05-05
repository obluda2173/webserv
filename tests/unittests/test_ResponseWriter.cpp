#include "ResponseWriter.h"
#include "gtest/gtest.h"
#include <gtest/gtest.h>

HttpResponse newResponse(std::string body) {
    HttpResponse resp;
    resp.statusCode = 200;
    resp.statusMessage = "OK";
    resp.body = new StringBodyProvider(body);
    resp.contentLength = body.length();
    resp.version = "HTTP/1.1";
    return resp;
}

typedef struct ResponseWriterParams {
    std::string body;
    size_t maxSize;
} ResponseWriterParams;

class ResponseWriterTest : public testing::TestWithParam<struct ResponseWriterParams> {};

TEST_P(ResponseWriterTest, firstTest) {
    ResponseWriterParams params = GetParam();

    char buffer[8192];
    HttpResponse resp = newResponse(params.body);

    size_t maxSize = params.maxSize;
    std::string want = "HTTP/1.1 200 OK\r\n"
                       "Content-Length: 4\r\n"
                       "\r\n"
                       "pong";

    IResponseWriter* wrtr = new ResponseWriter(resp);
    size_t writtenBytes = -1;
    std::string got = "";
    while (writtenBytes != 0) {
        writtenBytes = wrtr->write(buffer, maxSize);
        got += std::string(buffer, writtenBytes);
    }

    EXPECT_STREQ(want.c_str(), got.c_str());

    delete resp.body;
    delete wrtr;
}

INSTANTIATE_TEST_SUITE_P(maxSizes, ResponseWriterTest,
                         testing::Values(
                             // ResponseWriterParams{"pong", 1}, ResponseWriterParams{"pong", 2},
                             ResponseWriterParams{"pong", 3}));
