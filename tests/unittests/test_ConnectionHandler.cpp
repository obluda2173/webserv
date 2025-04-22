#include "ConnectionHandler.h"
#include "EpollIONotifier.h"
#include "IIONotifier.h"
#include "test_mocks.h"
#include "utils.h"
#include <cstddef>
#include <cstring>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <netdb.h>
#include <sys/socket.h>

TEST(ConnectionHandlerTest, firstTest) {
    MockLogger* logger = new MockLogger();
    IIONotifier* ioNotifier = new EpollIONotifier(*logger);
    ConnectionHandler* connHdlr = new ConnectionHandler(*logger, *ioNotifier);

    int serverfd = newListeningSocket(NULL, "8080", AF_INET6);
    struct addrinfo* svrAddrInfo;
    getSvrAddrInfo(NULL, "8080", AF_INET6, &svrAddrInfo);

    std::string clientIp = "00:00:00:00:00:00:00:01";
    std::string clientPort = "10001";

    int clientfd = newSocket(clientIp.c_str(), clientPort.c_str(), AF_INET6);
    ASSERT_NE(connect(clientfd, svrAddrInfo->ai_addr, svrAddrInfo->ai_addrlen), -1)
        << "connect: " << std::strerror(errno) << std::endl;
    freeaddrinfo(svrAddrInfo);

    EXPECT_CALL(*logger, log("INFO", "Connection accepted from IP: " + clientIp + ", Port: " + clientPort));
    connHdlr->handleConnection(serverfd);

    close(clientfd);
    close(serverfd);
    delete connHdlr;
    delete ioNotifier;
    delete logger;
}
