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

    int clientfd = getClientSocket("127.0.0.2", 8081);
    ASSERT_GT(clientfd, 0) << "getClientSocket failed";
    sockaddr_in svrAddr;
    setSvrAddr(svrAddr);
    EXPECT_CALL(mLogger, log("INFO", "Connection accepted from IP: 127.0.0.2, Port: 8081"));
    ASSERT_EQ(connect(clientfd, (sockaddr *)&svrAddr, sizeof(svrAddr)), 0) << "connect: " << strerror(errno);
    close(clientfd);

    clientfd = getClientSocket("127.0.0.2", 8082);
    ASSERT_GT(clientfd, 0) << "getClientSocket failed";
    setSvrAddr(svrAddr);
    EXPECT_CALL(mLogger, log("INFO", "Connection accepted from IP: 127.0.0.2, Port: 8081"));
    ASSERT_EQ(connect(clientfd, (sockaddr *)&svrAddr, sizeof(svrAddr)), 0) << "connect: " << strerror(errno);
    close(clientfd);

    EXPECT_CALL(mLogger, log("INFO", "Server is stopping..."));
    EXPECT_CALL(mLogger, log("INFO", "Server stopped"));
    svr.stop();

    serverThread.join();
}
