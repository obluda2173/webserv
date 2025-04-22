#include "test_ConnectionHandlerFixture.h"
#include "utils.h"
#include <cstddef>
#include <cstring>
#include <netdb.h>
#include <sys/socket.h>

TEST_F(ConnectionHdlrTestWithMockLogger, testLoggingIpV6) {
    struct addrinfo* svrAddrInfo;
    getAddrInfoHelper(NULL, "8080", AF_INET6, &svrAddrInfo);
    int serverfd = newListeningSocket(svrAddrInfo, 5);

    std::string clientIp = "00:00:00:00:00:00:00:01";
    std::string clientPort = "10001";
    int clientfd = newSocket(clientIp, clientPort, AF_INET6);

    ASSERT_NE(connect(clientfd, svrAddrInfo->ai_addr, svrAddrInfo->ai_addrlen), -1)
        << "connect: " << std::strerror(errno) << std::endl;

    EXPECT_CALL(*_logger, log("INFO", "Connection accepted from IP: " + clientIp + ", Port: " + clientPort));
    _connHdlr->handleConnection(serverfd);

    // freeaddrinfo(clientAddr);
    freeaddrinfo(svrAddrInfo);
    close(clientfd);
    close(serverfd);
}

// TEST_F(ConnectionHdlrTest, test1) {
//     addrinfo* svrAddrInfo;
//     getSvrAddrInfo(NULL, "8080", AF_INET, &svrAddrInfo);
//     int serverfd = newListeningSocket(NULL, "8080", AF_INET);

//     freeaddrinfo(svrAddrInfo);
//     close(serverfd);
// }
