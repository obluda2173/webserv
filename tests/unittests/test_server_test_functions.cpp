#include "test_main.h"
#include "test_mocks.h"
#include "utils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <netdb.h>
#include <random>
#include <thread>

void testOneConnectionWithLogging(MockLogger* mLogger, std::string& clientPort, std::string& clientIp,
                                  struct addrinfo* svrAddr) {
    struct addrinfo* clientAddrInfo;
    getAddrInfoHelper(clientIp.c_str(), clientPort.c_str(), AF_INET, &clientAddrInfo);
    int clientfd = newSocket(clientAddrInfo);
    freeaddrinfo(clientAddrInfo);
    ASSERT_GT(clientfd, 0) << "getClientSocket failed";
    EXPECT_CALL(*mLogger, log("INFO", "Connection accepted from IP: " + clientIp + ", Port: " + clientPort));
    ASSERT_EQ(connect(clientfd, svrAddr->ai_addr, svrAddr->ai_addrlen), 0) << "connect: " << strerror(errno);
    EXPECT_CALL(*mLogger, log("INFO", "Disconnect IP: " + clientIp + ", Port: " + clientPort));
    close(clientfd);
}

void testMultipleConnectionsWithLogging(MockLogger* mLogger, std::string svrPort, int nbrConns) {
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
        testOneConnectionWithLogging(mLogger, clientPort, clientIp, svrAddrInfo);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Add a delay after all connections to ensure all operations complete
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    freeaddrinfo(svrAddrInfo);
}

void testOneConnection(std::string& clientPort, std::string& clientIp, struct addrinfo* svrAddrInfo) {
    struct addrinfo* clientAddrInfo;
    getAddrInfoHelper(clientIp.c_str(), clientPort.c_str(), AF_INET, &clientAddrInfo);
    int clientfd = newSocket(clientAddrInfo);
    freeaddrinfo(clientAddrInfo);
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
        testOneConnection(clientPort, clientIp, svrAddrInfo);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    freeaddrinfo(svrAddrInfo);
}
