#include "IIONotifier.h"
#include "test_ConnectionHandlerFixture.h"
#include "utils.h"
#include "gmock/gmock.h"
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <netdb.h>
#include <sys/socket.h>
#include <thread>

TEST_F(ConnectionHdlrTestWithMockLoggerIPv6, acceptANewConnection) {
    std::string clientIp = "00:00:00:00:00:00:00:01";
    std::string clientPort = "10001";
    int clientfd = newSocket(clientIp, clientPort, AF_INET6);
    ASSERT_NE(connect(clientfd, _svrAddrInfo->ai_addr, _svrAddrInfo->ai_addrlen), -1)
        << "connect: " << std::strerror(errno) << std::endl;
    EXPECT_CALL(*_logger, log("INFO", "Connection accepted from IP: " + clientIp + ", Port: " + clientPort));
    _connHdlr->handleConnection(_serverfd, READY_TO_READ);
    close(clientfd);
}

TEST_F(ConnectionHdlrTest, pingTest) {
    char buffer[1024];
    std::string msg = "GET /ping HTTP/1.1\r\n\r\n";

    // send msg
    send(_clientfd, msg.c_str(), msg.length(), 0);
    _connHdlr->handleConnection(_conn, READY_TO_READ);

    // check that nothing is sent back yet
    recv(_clientfd, buffer, 1024, 0);
    ASSERT_EQ(errno, EWOULDBLOCK);

    // next time around the response is sent back
    _connHdlr->handleConnection(_conn, READY_TO_WRITE);
    ssize_t r = recv(_clientfd, buffer, 1024, 0);
    buffer[r] = '\0';
    std::string wantResponse = "HTTP/1.1 200 OK\r\n"
                               "Content-Length: 4\r\n"
                               "\r\n"
                               "pong";
    EXPECT_STREQ(buffer, wantResponse.c_str());
}

TEST_F(ConnectionHdlrTest, WrongRequest) {
    char buffer[1024];
    std::string msg = "GET \r\n\r\n";

    // send msg
    send(_clientfd, msg.c_str(), msg.length(), 0);
    _connHdlr->handleConnection(_conn, READY_TO_READ);

    // check that nothing is sent back yet
    recv(_clientfd, buffer, 1024, 0);
    ASSERT_EQ(errno, EWOULDBLOCK);

    // next time around the response is sent back
    _connHdlr->handleConnection(_conn, READY_TO_WRITE);
    ssize_t r = recv(_clientfd, buffer, 1024, 0);
    buffer[r] = '\0';
    std::string wantResponse = "HTTP/1.1 400 Bad Request\r\n"
                               "\r\n";
    EXPECT_STREQ(buffer, wantResponse.c_str());
}

TEST_F(ConnectionHdlrTest, pingTestInBatches) {
    char buffer[1024];
    std::string msg = "GET /ping HTTP/1.1\r\n\r\n";

    // cutting the msg into parts and send
    std::vector<std::string> chunks;
    for (std::size_t i = 0; i < msg.length(); i += 2) {
        send(_clientfd, msg.substr(i, 2).c_str(), msg.substr(i, 2).length(), 0);
        _connHdlr->handleConnection(_conn, READY_TO_READ);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        recv(_clientfd, buffer, 1024, 0);
        ASSERT_EQ(errno, EWOULDBLOCK);
    }

    // next time around the response is sent back
    _connHdlr->handleConnection(_conn, READY_TO_WRITE);
    ssize_t r = recv(_clientfd, buffer, 1024, 0);
    buffer[r] = '\0';
    std::string wantResponse = "HTTP/1.1 200 OK\r\n"
                               "Content-Length: 4\r\n"
                               "\r\n"
                               "pong";
    EXPECT_STREQ(buffer, wantResponse.c_str());
}

// TODO: the next two test do change nothing at the current code
// TEST_F(ConnectionHdlrTestWithMockLoggerIPv4, incompleteRequestThenClose) {
//     char buffer[1024];
//     std::string msg = "GET /ping HTT";

//     // send msg
//     send(_clientfd, msg.c_str(), msg.length(), 0);
//     _connHdlr->handleConnection(_conn, READY_TO_READ);

//     // check that nothing is sent back yet
//     recv(_clientfd, buffer, 1024, 0);
//     ASSERT_EQ(errno, EWOULDBLOCK);

//     // now connection is closed
//     EXPECT_CALL(*_logger, log("INFO", testing::HasSubstr("Disconnect IP")));
//     _connHdlr->handleConnection(_conn, CLIENT_HUNG_UP);
// }
