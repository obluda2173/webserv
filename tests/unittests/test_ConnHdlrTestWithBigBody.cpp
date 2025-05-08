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

    std::string gotResponse;
    int fd;
    e_notif notif;
    _ioNotifier->wait(&fd, &notif);
    while (notif == READY_TO_WRITE && fd != -1) {
        std::cout << "hello" << std::endl;
        _connHdlr->handleConnection(connfd, READY_TO_WRITE);
        char buffer[1025];
        ssize_t r = recv(clientfd, &buffer[0], 1024, 0);
        buffer[r] = '\0';
        gotResponse += buffer;
        _ioNotifier->wait(&fd, &notif);
        break;
    }

    std::string wantResponse = "HTTP/1.1 200 OK\r\n"
                               "Content-Length: " +
                               std::to_string(_body.length()) +
                               "\r\n"
                               "\r\n" +
                               _body;
    ASSERT_EQ(wantResponse, gotResponse);
    // ASSERT_EQ(_body.length(), gotBody.length());
}
