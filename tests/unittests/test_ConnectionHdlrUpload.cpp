#include "IIONotifier.h"
#include "test_ConnectionHandlerFixture.h"
#include "test_main.h"
#include <gtest/gtest.h>

TEST_F(ConnHdlrTestStubUploadHdlrSimple, testUpload) {
    int clientfd = _clientFdsAndConnFds[0].first;
    int connfd = _clientFdsAndConnFds[0].second;

    std::string body = getRandomString(100);
    std::string request = "POST /upload HTTP/1.1\r\n"
                          "Host: test.com\r\n"
                          "Content-Length: " +
                          std::to_string(body.length()) +
                          "\r\n"
                          "\r\n" +
                          body;

    send(clientfd, request.c_str(), request.length(), 0);
    _connHdlr->handleConnection(connfd, READY_TO_READ);
    ASSERT_EQ(body.length(), _uploadHdlr->_uploaded.length());
    ASSERT_EQ(body, _uploadHdlr->_uploaded);

    int fd;
    e_notif notif;
    _ioNotifier->wait(&fd, &notif);
    ASSERT_EQ(notif, READY_TO_WRITE);
    _connHdlr->handleConnection(connfd, READY_TO_WRITE);

    char buffer[1025];
    size_t r = recv(clientfd, &buffer[0], 1024, 0);
    buffer[r] = '\0';
    std::string gotResponse = buffer;
    std::string wantResponse = "HTTP/1.1 200 OK\r\n"
                               "\r\n";

    ASSERT_EQ(wantResponse, gotResponse);
}
