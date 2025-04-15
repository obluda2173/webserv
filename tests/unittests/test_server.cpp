#include "Server.h"
#include "test_main.h"
#include <cstring>
#include <netinet/in.h>
#include <random>
#include <string>

class MockLogger : public ILogger {
  public:
    MOCK_METHOD(void, log, (const std::string &level, const std::string &msg), (override));
};

// defining a Test Fixture: ServerTest
class ServerTest : public ::testing::Test {};

// void startSvr(Server &svr, MockLogger &mLogger) {
// }

void testMultipleConnections(MockLogger &mLogger) {
    std::random_device rd;                       // Obtain a random number from hardware
    std::mt19937 gen(rd());                      // Seed the generator
    std::uniform_int_distribution<> distr(1, 9); // Define the range

    int clientfd;
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

        clientfd = getClientSocket(clientIp, clientPort);
        ASSERT_GT(clientfd, 0) << "getClientSocket failed";
        EXPECT_CALL(mLogger,
                    log("INFO", "Connection accepted from IP: " + clientIp + ", Port: " + std::to_string(clientPort)));
        ASSERT_EQ(connect(clientfd, (sockaddr *)&svrAddr, sizeof(svrAddr)), 0) << "connect: " << strerror(errno);
        close(clientfd);
    }

    EXPECT_CALL(mLogger, log("INFO", "Server is stopping..."));
    EXPECT_CALL(mLogger, log("INFO", "Server stopped"));
}

TEST_F(ServerTest, connectionTest) {
    // setup
    MockLogger mLogger;
    EXPECT_CALL(mLogger, log("INFO", "Server constructed"));
    Server svr(&mLogger);

    EXPECT_CALL(mLogger, log("INFO", "Server is starting..."));
    EXPECT_CALL(mLogger, log("INFO", "Server started"));

    std::thread serverThread(&Server::start, &svr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_TRUE(svr.isRunning());

    // test some stuff
    testMultipleConnections(mLogger);

    // shutdown
    svr.stop();
    serverThread.join();
}
