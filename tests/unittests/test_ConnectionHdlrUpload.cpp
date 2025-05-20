#include "IIONotifier.h"
#include "test_ConnectionHandlerFixture.h"
#include "test_main.h"
#include "gtest/gtest.h"
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

    std::vector< t_notif > notifs = _ioNotifier->wait();
    ASSERT_EQ(notifs.size(), 1);
    ASSERT_EQ(notifs[0].notif, READY_TO_WRITE);
    _connHdlr->handleConnection(connfd, READY_TO_WRITE);

    char buffer[1025];
    size_t r = recv(clientfd, &buffer[0], 1024, 0);
    buffer[r] = '\0';
    std::string gotResponse = buffer;
    std::string wantResponse = "HTTP/1.1 200 OK\r\n"
                               "Content-Length: 0\r\n"
                               "\r\n";

    ASSERT_EQ(wantResponse, gotResponse);
}

TEST_F(ConnHdlrTestStubUploadHdlrSimple, testBigUpload) {
    int clientfd = _clientFdsAndConnFds[0].first;
    int connfd = _clientFdsAndConnFds[0].second;

    std::string body = getRandomString(9876); // clientMaxBody is 10000
    std::string request = "POST /upload HTTP/1.1\r\n"
                          "Host: test.com\r\n"
                          "Content-Length: " +
                          std::to_string(body.length()) +
                          "\r\n"
                          "\r\n" +
                          body;

    size_t pos = 0;
    size_t batchSize = 100;
    while (!checkNotification(_ioNotifier, t_notif{connfd, READY_TO_WRITE})) {
        if (pos > request.size())
            FAIL() << "position got over request length";
        std::string chunk = request.substr(pos, batchSize);
        send(clientfd, chunk.c_str(), chunk.length(), 0);
        pos += batchSize;

        _connHdlr->handleConnection(connfd, READY_TO_READ);
    }
    ASSERT_EQ(body.length(), _uploadHdlr->_uploaded.length());
    ASSERT_EQ(body, _uploadHdlr->_uploaded);

    std::vector< t_notif > notifs = _ioNotifier->wait();
    ASSERT_EQ(notifs.size(), 1);
    ASSERT_EQ(notifs[0].notif, READY_TO_WRITE);
    _connHdlr->handleConnection(connfd, READY_TO_WRITE);

    char buffer[1025];
    size_t r = recv(clientfd, &buffer[0], 1024, 0);
    buffer[r] = '\0';
    std::string gotResponse = buffer;
    std::string wantResponse = "HTTP/1.1 200 OK\r\n"
                               "Content-Length: 0\r\n"
                               "\r\n";

    ASSERT_EQ(wantResponse, gotResponse);
}
