#include "test_ConnectionHandlerFixture.h"
#include "test_main.h"
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <sys/types.h>

TEST_F(ConnHdlrTestRedirections, twoSmallSeperatedConsecutiveRequests) {
    // making consecutive requests assures that the body is properly handled
    int count = 0;
    while (count++ < 2) {
        int clientfd = _clientFdsAndConnFds[0].first;
        int connfd = _clientFdsAndConnFds[0].second;

        int contentLength = 100;
        std::string request = "GET /google HTTP/1.1\r\n"
                              "Host: test.com\r\n"
                              "Content-Length: " +
                              std::to_string(contentLength) +
                              "\r\n"
                              "\r\n" +
                              getRandomString(contentLength);

        send(clientfd, request.c_str(), request.length(), 0);
        readUntilREADY_TO_WRITE(_ioNotifier, _connHdlr, connfd);

        std::string gotResponse;
        std::vector< t_notif > notifs = _ioNotifier->wait();
        while (notifs.size() > 0 && notifs[0].notif == READY_TO_WRITE) {
            _connHdlr->handleConnection(connfd, READY_TO_WRITE);
            char buffer[1025];
            ssize_t r = recv(clientfd, &buffer[0], 1024, 0);
            buffer[r] = '\0';
            gotResponse += buffer;
            notifs = _ioNotifier->wait();
        }

        std::string wantResponse = "HTTP/1.1 301 Moved Permanently\r\n"
                                   "location: https://www.google.com\r\n"
                                   "Content-Length: 0\r\n"
                                   "\r\n";

        ASSERT_EQ(wantResponse, gotResponse);
        ASSERT_EQ(wantResponse.length(), gotResponse.length());
    }
}

TEST_F(ConnHdlrTestRedirections, twoBigConsecutiveRequests) {
    // making consecutive requests assures that the body is properly handled
    int clientfd = _clientFdsAndConnFds[0].first;
    int connfd = _clientFdsAndConnFds[0].second;

    int contentLength = 10000;
    std::string request = "GET /google HTTP/1.1\r\n"
                          "Host: test.com\r\n"
                          "Content-Length: " +
                          std::to_string(contentLength) +
                          "\r\n"
                          "\r\n" +
                          getRandomString(contentLength);

    request = request + request;
    std::cout << "send: " << send(clientfd, request.c_str(), request.length(), 0) << std::endl;

    int count = 0;
    while (count++ < 2) {
        readUntilREADY_TO_WRITE(_ioNotifier, _connHdlr, connfd);

        std::string gotResponse = "";
        std::vector< t_notif > notifs = _ioNotifier->wait();

        std::cout << std::endl << "calling ready to write" << std::endl;
        _connHdlr->handleConnection(connfd, READY_TO_WRITE);
        char buffer[1025];
        ssize_t r = recv(clientfd, &buffer[0], 1024, 0);
        buffer[r] = '\0';
        gotResponse += buffer;
        // notifs = _ioNotifier->wait();

        std::string wantResponse = "HTTP/1.1 301 Moved Permanently\r\n"
                                   "location: https://www.google.com\r\n"
                                   "Content-Length: 0\r\n"
                                   "\r\n";

        ASSERT_EQ(wantResponse, gotResponse);
        ASSERT_EQ(wantResponse.length(), gotResponse.length());
    }
}
