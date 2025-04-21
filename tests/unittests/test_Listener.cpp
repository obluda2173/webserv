#include "test_ListenerFixtures.h"
#include "test_main.h"
#include "utils.h"
#include "gtest/gtest.h"
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

TEST_P(ListenerTestWithMockLogging, closingAConnection) {
    std::vector<int> ports = GetParam();
    for (size_t i = 0; i < ports.size(); i++) {
        int port = ports[i];
        std::string clientPort = "12345";
        std::string clientIp = "127.0.0.3";
        int clientfd = newSocket(clientIp.c_str(), clientPort.c_str(), AF_INET);
        struct addrinfo* svrAddrInfo;
        getSvrAddrInfo(NULL, std::to_string(port).c_str(), AF_INET, &svrAddrInfo);

        EXPECT_CALL(*_logger, log("INFO", "Connection accepted from IP: " + clientIp + ", Port: " + clientPort));
        ASSERT_EQ(connect(clientfd, svrAddrInfo->ai_addr, svrAddrInfo->ai_addrlen), 0)
            << "connect: " << strerror(errno);
        freeaddrinfo(svrAddrInfo);

        EXPECT_CALL(*_logger, log("INFO", "Disconnect IP: " + clientIp + ", Port: " + clientPort));
        close(clientfd);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

TEST_P(ListenerTestWithMockLogging, multipleConnections) {
    std::vector<int> ports = GetParam();
    for (size_t i = 0; i < ports.size(); i++) {
        int port = ports[i];
        testMultipleConnectionsWithLogging(_logger, std::to_string(port), 50);
    }
}

INSTANTIATE_TEST_SUITE_P(multiplePorts, ListenerTestWithMockLogging,
                         ::testing::Values(std::vector<int>{8080}, std::vector<int>{8080, 8081}));

TEST_P(ListenerTestWoMockLogging, multiplePortsTestWoLogging) {
    std::vector<int> ports = GetParam();
    for (size_t i = 0; i < ports.size(); i++) {
        int port = ports[i];
        testMultipleConnections(std::to_string(port), 50);
    }
}

INSTANTIATE_TEST_SUITE_P(multiplePorts, ListenerTestWoMockLogging,
                         ::testing::Values(std::vector<int>{8080}, std::vector<int>{8080, 8081}));
