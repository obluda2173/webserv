#include "test_fixtures.h"
#include "test_main.h"
#include <random>

TEST_F(ServerTest, connectionTest) { testMultipleConnections(_mLogger); }

void testOneConnection(MockLogger& mLogger, int& clientPort, std::string& clientIp, sockaddr_in svrAddr) {
    int clientfd = getClientSocket(clientIp, clientPort);
    ASSERT_GT(clientfd, 0) << "getClientSocket failed";
    EXPECT_CALL(mLogger,
                log("INFO", "Connection accepted from IP: " + clientIp + ", Port: " + std::to_string(clientPort)));
    ASSERT_EQ(connect(clientfd, (sockaddr*)&svrAddr, sizeof(svrAddr)), 0) << "connect: " << strerror(errno);
    close(clientfd);
}

void testMultipleConnections(MockLogger& mLogger) {
    std::random_device rd;                       // Obtain a random number from hardware
    std::mt19937 gen(rd());                      // Seed the generator
    std::uniform_int_distribution<> distr(1, 9); // Define the range

    int count = 0;
    int nbrConns = 1000;
    int clientPort = 8080;
    std::string clientIp = "127.0.0.";
    sockaddr_in svrAddr;
    setSvrAddr(svrAddr);

    while (count++ < nbrConns) {
        int randNbr = distr(gen); // Generate the random number
        clientPort += randNbr;
        clientIp = "127.0.0." + std::to_string(randNbr);
        testOneConnection(mLogger, clientPort, clientIp, svrAddr);
    }
}
