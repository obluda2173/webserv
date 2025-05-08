#include "test_ConnectionHandlerFixture.h"
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <sys/types.h>

// checking if the body is sent over the connection entirely
TEST_F(ConnHdlrTestWithBigBody, firstTest) {
    int clientfd = _clientFdsAndConnFds[0].first;
    int connfd = _clientFdsAndConnFds[0].second;

    std::string request = "GET / HTTP/1.1\r\n"
                          "Host: test.com\r\n"
                          "\r\n";

    send(clientfd, request.c_str(), request.length(), 0);
    readUntilREADY_TO_WRITE(_ioNotifier, _connHdlr, connfd);

    _connHdlr->handleConnection(connfd, READY_TO_WRITE);
    char buffer[1025];

    ssize_t r = recv(clientfd, &buffer[0], 1024, 0);
    buffer[r] = '\0';
    std::string gotBody = buffer;
    ASSERT_EQ(_body.substr(1025), gotBody);
    // ASSERT_EQ(_body.length(), gotBody.length());
}
