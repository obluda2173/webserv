#include "test_ConnectionHandlerFixture.h"

TEST_F(ConnHdlrTestTestRouting, firstTest) {
    int clientfd = _clientFdsAndConnFds[0].first;
    int connfd = _clientFdsAndConnFds[0].second;

    std::string request = "GET /notImages/../images/ping HTTP/1.1\r\n"
                          "Host: test.com\r\n"
                          "\r\n";

    send(clientfd, request.c_str(), request.length(), 0);
    readUntilREADY_TO_WRITE(_ioNotifier, _connHdlr, connfd);
}

TEST_F(ConnHdlrTestTestRouting, secondTest) {
    int clientfd = _clientFdsAndConnFds[0].first;
    int connfd = _clientFdsAndConnFds[0].second;

    std::string request = "GET /notPresent HTTP/1.1\r\n"
                          "Host: test.com\r\n"
                          "\r\n";
    std::string wantResponse = "HTTP/1.1 404 Not Found\r\n"
                               "Content-Type: text/plain\r\n"
                               "Content-Language: en-US\r\n"
                               "Content-Length: 9\r\n"
                               "\r\n"
                               "Not Found";

    send(clientfd, request.c_str(), request.length(), 0);
    readUntilREADY_TO_WRITE(_ioNotifier, _connHdlr, connfd);

    char buffer[1024];
    // next time around the response is sent back
    _connHdlr->handleConnection(connfd, READY_TO_WRITE);
    ssize_t r = recv(clientfd, buffer, 1024, 0);
    buffer[r] = '\0';
    EXPECT_STREQ(buffer, wantResponse.c_str());
}
