#include "Server.h"
#include "test_main.h"
#include <cstring>

class MockLogger : public ILogger {
  public:
    MOCK_METHOD(void, log, (const std::string &level, const std::string &msg), (override));
};

// defining a Test Fixture: ServerTest
class ServerTest : public ::testing::Test {};

// void startSvr(Server &svr, MockLogger &mLogger) {
// }

TEST_F(ServerTest, connectionTest) {
    MockLogger mLogger;
    EXPECT_CALL(mLogger, log("INFO", "Server constructed"));
    Server svr(&mLogger);
    // startSvr(svr, mLogger);

    EXPECT_CALL(mLogger, log("INFO", "Server is starting..."));
    EXPECT_CALL(mLogger, log("INFO", "Server started"));
    std::thread serverThread(&Server::start, &svr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_TRUE(svr.isRunning());

    // std::string clientIp;
    int clientPort;
    clientPort = 8080;

    int count = 0;
    while (count++ < 3) {
        int clientfd = getClientSocket("127.0.0.1", ++clientPort);
        ASSERT_GT(clientfd, 0) << "getClientSocket failed";
        sockaddr_in svrAddr;
        setSvrAddr(svrAddr);
        EXPECT_CALL(mLogger,
                    log("INFO", "Connection accepted from IP: 127.0.0.1, Port: " + std::to_string(clientPort)));
        ASSERT_EQ(connect(clientfd, (sockaddr *)&svrAddr, sizeof(svrAddr)), 0) << "connect: " << strerror(errno);
        close(clientfd);
    }

    EXPECT_CALL(mLogger, log("INFO", "Server is stopping..."));
    EXPECT_CALL(mLogger, log("INFO", "Server stopped"));
    svr.stop();

    serverThread.join();
}
