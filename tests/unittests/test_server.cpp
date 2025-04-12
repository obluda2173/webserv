#include "Logger.h"
#include "Server.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <sys/socket.h>
#include <thread>

class MockLogger : public ILogger {
  public:
    MOCK_METHOD(void, log, (const std::string &level, const std::string &msg), (override));
};

// defining a Test Fixture: ServerTest
class ServerTest : public ::testing::Test {};

TEST_F(ServerTest, startupTest) {
    MockLogger mLogger;

    EXPECT_CALL(mLogger, log("INFO", "Server constructed"));
    EXPECT_CALL(mLogger, log("INFO", "Server is running"));
    Server svr(&mLogger);

    EXPECT_FALSE(svr.isRunning());
    svr.start();
    EXPECT_TRUE(svr.isRunning());
}

TEST_F(ServerTest, shutdownTest) {
    MockLogger mLogger;
    EXPECT_CALL(mLogger, log("INFO", "Server constructed"));
    EXPECT_CALL(mLogger, log("INFO", "Server is running"));
    EXPECT_CALL(mLogger, log("INFO", "Server stopped"));

    Server svr(&mLogger);
    svr.start();
    svr.stop();
    EXPECT_FALSE(svr.isRunning());
}

TEST_F(ServerTest, connectionTests) {
    Logger logger;
    testing::internal::CaptureStdout();
    Server svr(&logger);

    std::thread svr_thread(&Server::start, &svr);
    svr_thread.join();

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_TRUE(client_socket >= 0);
}
