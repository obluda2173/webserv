#include "test_fixtures.h"
#include "test_main.h"
#include <random>

TEST_F(ServerTest, connectionTest) { testMultipleConnections(_mLogger, 8080); }

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
    std::uniform_int_distribution<> distr1(8081, 20000); // Define the range
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
