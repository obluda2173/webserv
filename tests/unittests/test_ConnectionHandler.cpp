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

TEST_F(ConnectionHdlrTest, test1) {
    int clientfd = newSocket("127.0.0.2", "12345", AF_INET);
    ASSERT_NE(connect(clientfd, _svrAddrInfo->ai_addr, _svrAddrInfo->ai_addrlen), -1)
        << "connect: " << std::strerror(errno) << std::endl;

    int conn = _connHdlr->handleConnection(_serverfd, READY_TO_READ);

    char buffer[1024];
    // set to non-blocking to make sure that nothing is send after first handleConnectionCall
    fcntl(clientfd, F_SETFL, O_NONBLOCK);
    send(clientfd, "some bytes", 10, 0);
    _connHdlr->handleConnection(conn, READY_TO_READ);

    recv(clientfd, buffer, 1024, 0);
    ASSERT_EQ(errno, EWOULDBLOCK);

    _connHdlr->handleConnection(conn, READY_TO_WRITE);
    ssize_t r = recv(clientfd, buffer, 1024, 0);
    buffer[r] = '\0';
    EXPECT_STREQ(buffer, "some bytes, some other bytes");
}
