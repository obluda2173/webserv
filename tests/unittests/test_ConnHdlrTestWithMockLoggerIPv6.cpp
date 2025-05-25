#include "test_ConnectionHandlerFixture.h"

TEST_F(ConnHdlrTestWithMockLoggerIPv6, acceptANewConnection) {
    using ::testing::_;
    EXPECT_CALL(*_logger, log(_, _)).Times(testing::AnyNumber());
    std::string clientIp = "00:00:00:00:00:00:00:01";
    std::string clientPort = "10001";
    int clientfd = newSocket(clientIp, clientPort, AF_INET6);
    ASSERT_NE(connect(clientfd, _svrAddrInfo->ai_addr, _svrAddrInfo->ai_addrlen), -1)
        << "connect: " << std::strerror(errno) << std::endl;
    EXPECT_CALL(*_logger, log("INFO", "Connection accepted from IP: " + clientIp + ", Port: " + clientPort));
    _connHdlr->handleConnection(_serverfd, READY_TO_READ);
    close(clientfd);
}
