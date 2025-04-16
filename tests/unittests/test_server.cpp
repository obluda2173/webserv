#include "test_fixtures.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <random>

TEST_P(ServerTest, connectionTest) {
    std::vector<int> listeningPorts = GetParam();
    for (size_t i = 0; i < listeningPorts.size(); i++) {
        testMultipleConnections(_mLogger, listeningPorts[i]);
    }
}

INSTANTIATE_TEST_SUITE_P(ServerTests, ServerTest,
                         ::testing::Values(std::vector<int>{8080}, std::vector<int>{8080, 8081},
                                           std::vector<int>{8080, 8081, 8082}));

void testOneConnection(MockLogger& mLogger, int& clientPort, std::string& clientIp, sockaddr_in svrAddr) {
    int clientfd = getClientSocket(clientIp, clientPort);
    ASSERT_GT(clientfd, 0) << "getClientSocket failed";
    EXPECT_CALL(mLogger,
                log("INFO", "Connection accepted from IP: " + clientIp + ", Port: " + std::to_string(clientPort)));
    ASSERT_EQ(connect(clientfd, (sockaddr*)&svrAddr, sizeof(svrAddr)), 0) << "connect: " << strerror(errno);
    close(clientfd);
}

void testMultipleConnections(MockLogger& mLogger, int port) {
    std::random_device rd;                               // Obtain a random number from hardware
    std::mt19937 gen(rd());                              // Seed the generator
    std::uniform_int_distribution<> distr1(9000, 20000); // Define the range
    std::uniform_int_distribution<> distr2(1, 255);      // Define the range

    int clientPort;
    std::string clientIp;
    sockaddr_in svrAddr;
    setSvrAddr(svrAddr, port);
    int count = 0;
    int nbrConns = 1000;
    while (count++ < nbrConns) {
        clientPort = distr1(gen);
        clientIp = "127.0.0." + std::to_string(distr2(gen));
        testOneConnection(mLogger, clientPort, clientIp, svrAddr);
    }
}
