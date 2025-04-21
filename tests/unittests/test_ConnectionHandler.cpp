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

TEST(ConnectionHandlerTest, firstTest) {
    MockLogger* logger = new MockLogger();
    IIONotifier* ioNotifier = new EpollIONotifier(*logger);
    ConnectionHandler* connHdlr = new ConnectionHandler(*logger, *ioNotifier);

    int serverfd = newListeningSocket(NULL, "8080");

    std::string clientIp = "::1";
    std::string clientPort = "10001";

    // char ipstr[INET6_ADDRSTRLEN]; /* to store the ip-address */
    // struct addrinfo* addrInfo;
    // getSvrAddrInfo(clientIp.c_str(), clientPort.c_str(), &addrInfo);

    // void* addr;
    // const char* ipver;
    // struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)addrInfo->ai_addr;
    // addr = &(ipv6->sin6_addr);
    // ipver = "IPv6";

    // inet_ntop(addrInfo->ai_family, addr, ipstr, sizeof ipstr);
    // printf("  %s: %s\n", ipver, ipstr);

    struct addrinfo* svrAddrInfo;
    int status;
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((status = getaddrinfo(NULL, "8080", &hints, &svrAddrInfo) == -1))
        std::cerr << "getaddrinfo error: " << gai_strerror(status);

    if (svrAddrInfo == NULL) {
        std::cout << "yep" << std::endl;
        exit(1);
    }

    int clientfd = newSocket(clientIp.c_str(), clientPort.c_str());
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
