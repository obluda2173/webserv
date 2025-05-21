#include "Connection.h"
#include "HttpResponse.h"
#include "test_main.h"
#include <gtest/gtest.h>
#include <random>

size_t random_up_to(size_t size) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution< std::size_t > dist(0, size);
    return dist(gen);
}

class MockSender : public ISender {
  public:
    std::string response;
    virtual size_t _send(int fd, char* buf, size_t size) override {
        (void)fd;
        size_t randomNbr = random_up_to(size);
        response += std::string(buf, randomNbr);
        return randomNbr;
    }
};

TEST(ConnectionTestChunkedSend, testPartiallySending) {
    MockSender* sender = new MockSender();
    Connection conn({}, -1, "", NULL, sender);
    std::string body = getRandomString(10000);
    std::string wantResponse = "HTTP/1.1 200 OK\r\n"
                               "Content-Length: " +
                               std::to_string(body.length()) +
                               "\r\n"
                               "\r\n" +
                               body;

    conn._response.statusCode = 200;
    conn._response.version = "HTTP/1.1";
    conn._response.statusMessage = "OK";
    conn._response.contentLength = body.length();
    conn._response.body = new StringBodyProvider(body);
    int count = 0; // to make the test terminate
    while (sender->response.length() < wantResponse.length() && count++ < 50)
        conn.sendResponse();
    ASSERT_EQ(sender->response.length(), wantResponse.length());
    ASSERT_EQ(sender->response, wantResponse);
}
