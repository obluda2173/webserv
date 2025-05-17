#include "test_ConnectionHandlerFixture.h"

TEST_F(ConnHdlrTestTestRouting, firstTest) {
    int clientfd = _clientFdsAndConnFds[0].first;
    int connfd = _clientFdsAndConnFds[0].second;

    std::string request = "GET /notImages/../images/ping HTTP/1.1\r\n"
                          "Host: test.com\r\n"
                          "\r\n";

    std::string wantResponse = "HTTP/1.1 200 OK\r\n"
                               "Content-Length: 4\r\n"
                               "\r\n"
                               "pong";

    send(clientfd, request.c_str(), request.length(), 0);
    readUntilREADY_TO_WRITE(_ioNotifier, _connHdlr, connfd);

    // recv(clientfd, buffer, 1024, 0);
    // ASSERT_EQ(errno, EWOULDBLOCK);

    // // next time around the response is sent back
    // _connHdlr->handleConnection(connfd, READY_TO_WRITE);
    // ssize_t r = recv(clientfd, buffer, 1024, 0);
    // buffer[r] = '\0';
    // EXPECT_STREQ(buffer, wantResponse.c_str());
}
