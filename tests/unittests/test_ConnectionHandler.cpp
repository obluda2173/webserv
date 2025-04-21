#include "ConnectionHandler.h"
#include "EpollIONotifier.h"
#include "IIONotifier.h"
#include "test_mocks.h"
#include "utils.h"
#include <cstddef>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <netdb.h>

TEST(ConnectionHandlerTest, firstTest) {
    MockLogger* logger = new MockLogger();
    IIONotifier* ioNotifier = new EpollIONotifier(*logger);
    ConnectionHandler* connHdlr = new ConnectionHandler(*logger, *ioNotifier);

    struct addrinfo* svrAddrInfo;
    getSvrAddrInfo(NULL, "8080", &svrAddrInfo);
    int serverfd = newListeningSocket(NULL, "8080");

    std::string clientIp = "127.0.0.2";
    std::string clientPort = "10001";
    int clientfd = newSocket(clientIp.c_str(), clientPort.c_str());
    ASSERT_NE(connect(clientfd, svrAddrInfo->ai_addr, svrAddrInfo->ai_addrlen), -1);
    freeaddrinfo(svrAddrInfo);

    EXPECT_CALL(*logger, log("INFO", "Connection accepted from IP: " + clientIp + ", Port: " + clientPort));
    connHdlr->handleConnection(serverfd);

    close(clientfd);
    close(serverfd);
    delete connHdlr;
    delete ioNotifier;
    delete logger;
}
