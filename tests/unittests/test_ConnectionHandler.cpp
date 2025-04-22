#include "test_ConnectionHandlerFixture.h"
#include "utils.h"
#include <cstddef>
#include <cstring>
#include <netdb.h>
#include <sys/socket.h>

TEST_F(ConnectionHdlrTestWithMockLogger, testLoggingIpV6) {
    int serverfd = newListeningSocket(NULL, "8080", AF_INET6);
    struct addrinfo* svrAddrInfo;
    getSvrAddrInfo(NULL, "8080", AF_INET6, &svrAddrInfo);

    std::string clientIp = "00:00:00:00:00:00:00:01";
    std::string clientPort = "10001";

    int clientfd = newSocket(clientIp.c_str(), clientPort.c_str(), AF_INET6);
    ASSERT_NE(connect(clientfd, svrAddrInfo->ai_addr, svrAddrInfo->ai_addrlen), -1)
        << "connect: " << std::strerror(errno) << std::endl;
    freeaddrinfo(svrAddrInfo);

    EXPECT_CALL(*_logger, log("INFO", "Connection accepted from IP: " + clientIp + ", Port: " + clientPort));
    _connHdlr->handleConnection(serverfd);

    close(clientfd);
    close(serverfd);
}
