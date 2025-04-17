#include "test_fixtures.h"
#include "test_main.h"
#include <random>

void testOneConnectionWithLogging(MockLogger* mLogger, int& clientPort, std::string& clientIp, sockaddr_in svrAddr) {
    int clientfd = getClientSocket(clientIp, clientPort);
    ASSERT_GT(clientfd, 0) << "getClientSocket failed";
    EXPECT_CALL(*mLogger,
                log("INFO", "Connection accepted from IP: " + clientIp + ", Port: " + std::to_string(clientPort)));
    ASSERT_EQ(connect(clientfd, (sockaddr*)&svrAddr, sizeof(svrAddr)), 0) << "connect: " << strerror(errno);
    EXPECT_CALL(*mLogger, log("INFO", "Disconnect IP: " + clientIp + ", Port: " + std::to_string(clientPort)));
    close(clientfd);
}

void testMultipleConnectionsWithLogging(MockLogger* mLogger, int port, int nbrConns) {
    std::random_device rd;                               // Obtain a random number from hardware
    std::mt19937 gen(rd());                              // Seed the generator
    std::uniform_int_distribution<> distr1(9000, 20000); // Define the range
    std::uniform_int_distribution<> distr2(1, 255);      // Define the range

    int clientPort;
    std::string clientIp;
    sockaddr_in svrAddr;
    setSvrAddr(svrAddr, port);
    int count = 0;
    while (count++ < nbrConns) {
        clientPort = distr1(gen);
        clientIp = "127.0.0." + std::to_string(distr2(gen));
        testOneConnectionWithLogging(mLogger, clientPort, clientIp, svrAddr);
    }
}

void testOneConnection(int& clientPort, std::string& clientIp, sockaddr_in svrAddr) {
    int clientfd = getClientSocket(clientIp, clientPort);
    ASSERT_GT(clientfd, 0) << "getClientSocket failed";
    ASSERT_EQ(connect(clientfd, (sockaddr*)&svrAddr, sizeof(svrAddr)), 0) << "connect: " << strerror(errno);
    close(clientfd);
}

void testMultipleConnections(int port, int nbrConns) {
    std::random_device rd;                               // Obtain a random number from hardware
    std::mt19937 gen(rd());                              // Seed the generator
    std::uniform_int_distribution<> distr1(9000, 20000); // Define the range
    std::uniform_int_distribution<> distr2(1, 255);      // Define the range

    int clientPort;
    std::string clientIp;
    sockaddr_in svrAddr;
    setSvrAddr(svrAddr, port);
    int count = 0;
    while (count++ < nbrConns) {
        clientPort = distr1(gen);
        clientIp = "127.0.0." + std::to_string(distr2(gen));
        testOneConnection(clientPort, clientIp, svrAddr);
    }
}
