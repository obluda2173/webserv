#include "test_main.h"
#include "test_mocks.h"
#include "utils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <netdb.h>
#include <random>
#include <sys/socket.h>
#include <thread>

void testOneConnectionWithLogging(testing::NiceMock< MockLogger >* mLogger, std::string& clientIp,
                                  std::string& clientPort, struct addrinfo* svrAddr) {

    int clientfd = newSocket(clientIp, clientPort, AF_INET);
    ASSERT_GT(clientfd, 0) << "getClientSocket failed";
    EXPECT_CALL(*mLogger, log("INFO", "Connection accepted from IP: " + clientIp + ", Port: " + clientPort));
    ASSERT_EQ(connect(clientfd, svrAddr->ai_addr, svrAddr->ai_addrlen), 0) << "connect: " << strerror(errno);
    EXPECT_CALL(*mLogger, log("INFO", "Disconnect IP: " + clientIp + ", Port: " + clientPort));
    close(clientfd);
}

void testMultipleConnectionsWithLogging(testing::NiceMock< MockLogger >* mLogger, std::string svrPort, int nbrConns) {
    std::random_device rd;                               // Obtain a random number from hardware
    std::mt19937 gen(rd());                              // Seed the generator
    std::uniform_int_distribution<> distr1(9000, 20000); // Define the range
    std::uniform_int_distribution<> distr2(1, 255);      // Define the range

    std::string clientPort;
    std::string clientIp;

    struct addrinfo* svrAddrInfo;
    getAddrInfoHelper(NULL, svrPort.c_str(), AF_INET, &svrAddrInfo);
    int count = 0;
    while (count++ < nbrConns) {
        clientPort = std::to_string(distr1(gen));
        clientIp = "127.0.0." + std::to_string(distr2(gen));
        testOneConnectionWithLogging(mLogger, clientIp, clientPort, svrAddrInfo);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    // Add a delay after all connections to ensure all operations complete
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    freeaddrinfo(svrAddrInfo);
}

void testOneConnection(std::string& clientIp, std::string& clientPort, struct addrinfo* svrAddrInfo) {
    int clientfd = newSocket(clientIp, clientPort, AF_INET);
    ASSERT_GT(clientfd, 0) << "getClientSocket failed";
    ASSERT_EQ(connect(clientfd, svrAddrInfo->ai_addr, svrAddrInfo->ai_addrlen), 0) << "connect: " << strerror(errno);
    close(clientfd);
}

void testMultipleConnections(std::string svrPort, int nbrConns) {
    std::random_device rd;                               // Obtain a random number from hardware
    std::mt19937 gen(rd());                              // Seed the generator
    std::uniform_int_distribution<> distr1(9000, 20000); // Define the range
    std::uniform_int_distribution<> distr2(1, 255);      // Define the range

    std::string clientPort;
    std::string clientIp;

    struct addrinfo* svrAddrInfo;
    getAddrInfoHelper(NULL, svrPort.c_str(), AF_INET, &svrAddrInfo);
    int count = 0;
    while (count++ < nbrConns) {
        clientPort = std::to_string(distr1(gen));
        clientIp = "127.0.0." + std::to_string(distr2(gen));
        testOneConnection(clientIp, clientPort, svrAddrInfo);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    freeaddrinfo(svrAddrInfo);
}
