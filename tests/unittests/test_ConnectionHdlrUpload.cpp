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

    std::vector< t_notif > notifs = _ioNotifier->waitVector();
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

int StubUploadHdlrAdvanced::_nbrOfHandleCallsUntilUploadComplete = 3;
TEST_F(ConnHdlrTestStubUploadHdlrAdvanced, testUpload) {
    int nbrOfHandleCallsUntilUploadComplete = StubUploadHdlrAdvanced::_nbrOfHandleCallsUntilUploadComplete;
    int clientfd = _clientFdsAndConnFds[0].first;
    int connfd = _clientFdsAndConnFds[0].second;

    std::string body = getRandomString(100);
    std::string requestHeaders = "POST /upload HTTP/1.1\r\n"
                                 "Host: test.com\r\n"
                                 "Content-Length: " +
                                 std::to_string(body.length()) +
                                 "\r\n"
                                 "\r\n";

    send(clientfd, requestHeaders.c_str(), requestHeaders.length(), 0);
    _connHdlr->handleConnection(connfd, READY_TO_READ);

    int calls = 0;
    while (calls < nbrOfHandleCallsUntilUploadComplete) {
        send(clientfd, body.data(), body.length(), 0);
        verifyNotification(_ioNotifier, t_notif{connfd, READY_TO_READ});
        _connHdlr->handleConnection(connfd, READY_TO_READ);
        calls++;
    }

    std::vector< t_notif > notifs = _ioNotifier->waitVector();
    ASSERT_EQ(notifs.size(), 1);
    ASSERT_EQ(notifs[0].notif, READY_TO_WRITE);
}
