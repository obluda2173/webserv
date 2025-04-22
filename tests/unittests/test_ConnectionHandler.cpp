#include "IIONotifier.h"
#include "test_ConnectionHandlerFixture.h"
#include "utils.h"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <netdb.h>
#include <sys/socket.h>

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

// TEST_F(ConnectionHdlrTest, incompleteRequest) {
//     char buffer[1024];
//     std::string msg = "GET /ping HTT";

//     // send msg
//     send(_clientfd, msg.c_str(), msg.length(), 0);
//     _connHdlr->handleConnection(_conn, READY_TO_READ);

//     // check that nothing is sent back yet
//     recv(_clientfd, buffer, 1024, 0);
//     ASSERT_EQ(errno, EWOULDBLOCK);

//     // next time around the response is sent back
//     _connHdlr->handleConnection(_conn, READY_TO_WRITE);
//     ssize_t r = recv(_clientfd, buffer, 1024, 0);
//     buffer[r] = '\0';
//     std::string wantResponse = "HTTP/1.1 200 OK\r\n"
//                                "Content-Length: 4\r\n"
//                                "\r\n"
//                                "pong";
//     EXPECT_STREQ(buffer, wantResponse.c_str());
// }
