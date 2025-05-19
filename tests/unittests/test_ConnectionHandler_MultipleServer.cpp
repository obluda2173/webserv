#include "test_ConnectionHandler_MultipleServerFixture.h"
#include "gtest/gtest.h"

TEST_F(ConnHdlrTestMultipleRouter, TwoRouters) {
    std::string request = "GET /ping HTTP/1.1\r\n"
                          "Host: test.com\r\n"
                          "\r\n";
    std::string wantResponse8080 = "HTTP/1.1 200 OK\r\n"
                                   "Content-Length: 4\r\n"
                                   "\r\n"
                                   "pong";

    std::string wantResponse8081 = "HTTP/1.1 200 OK\r\n"
                                   "Content-Length: 24\r\n"
                                   "\r\n"
                                   "a totally different body";

    char buffer[1024];
    send(_clientfd8080, request.c_str(), request.length(), 0);
    readUntilREADY_TO_WRITE(_ioNotifier, _connHdlr, _connfd8080);
    ASSERT_TRUE(checkNotification(_ioNotifier, t_notif{_connfd8080, READY_TO_WRITE}));
    // check that nothing is sent back yet
    recv(_clientfd8080, buffer, 1024, 0);
    ASSERT_EQ(errno, EWOULDBLOCK);

    _connHdlr->handleConnection(_connfd8080, READY_TO_WRITE);
    ssize_t r = recv(_clientfd8080, buffer, 1024, 0);
    buffer[r] = '\0';

    EXPECT_STREQ(buffer, wantResponse8080.c_str());

    send(_clientfd8081, request.c_str(), request.length(), 0);
    readUntilREADY_TO_WRITE(_ioNotifier, _connHdlr, _connfd8081);
    ASSERT_TRUE(checkNotification(_ioNotifier, t_notif{_connfd8081, READY_TO_WRITE}));
    // check that nothing is sent back yet
    recv(_clientfd8081, buffer, 1024, 0);
    ASSERT_EQ(errno, EWOULDBLOCK);

    _connHdlr->handleConnection(_connfd8081, READY_TO_WRITE);
    r = recv(_clientfd8081, buffer, 1024, 0);
    buffer[r] = '\0';

    EXPECT_STREQ(buffer, wantResponse8081.c_str());
}
