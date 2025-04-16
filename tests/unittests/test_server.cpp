#include "Listener.h"
#include "Server.h"
#include "test_main.h"
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <netinet/in.h>
#include <random>
#include <string>

class MockLogger : public ILogger {
  public:
    MOCK_METHOD(void, log, (const std::string& level, const std::string& msg), (override));
};

int countOpenFileDescriptors() {
    int count = 0;
    DIR* dir = opendir("/proc/self/fd");
    if (dir != nullptr) {
        while (readdir(dir) != nullptr) {
            count++;
        }
        closedir(dir);
        // Adjust count to exclude ".", ".." and the dir fd itself
        count -= 3;
    }
    return count;
}

// defining a Test Fixture: ServerTest
class ServerTest : public ::testing::Test {
  protected:
    MockLogger mLogger;
    Server svr;
    std::thread serverThread;
    int openFdsBegin;

    ServerTest() : svr(&mLogger) {}

    void SetUp() override { setupServer(); }

    void TearDown() override { teardownServer(); }

    void setupServer() {
        openFdsBegin = countOpenFileDescriptors();
        EXPECT_CALL(mLogger, log("INFO", "Server is starting..."));
        EXPECT_CALL(mLogger, log("INFO", "Server started"));

        serverThread = std::thread(&Server::start, &svr);
        waitForServerStartup();
    }

    void waitForServerStartup() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        EXPECT_TRUE(svr.isRunning());
    }

    void teardownServer() {
        EXPECT_CALL(mLogger, log("INFO", "Server is stopping..."));
        EXPECT_CALL(mLogger, log("INFO", "Server stopped"));

        svr.stop();
        EXPECT_FALSE(svr.isRunning());
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        EXPECT_EQ(openFdsBegin, countOpenFileDescriptors());
        serverThread.join();
    }
};

TEST_F(ServerTest, connectionTest) { testMultipleConnections(mLogger); }

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
