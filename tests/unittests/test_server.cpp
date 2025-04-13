#include "Server.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
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
    EXPECT_CALL(mLogger, log("INFO", "Server started"));
    Server svr(&mLogger);

    EXPECT_FALSE(svr.isRunning());
    svr.start();
    EXPECT_TRUE(svr.isRunning());
}

TEST_F(ServerTest, shutdownTest) {
    MockLogger mLogger;
    EXPECT_CALL(mLogger, log("INFO", "Server constructed"));
    EXPECT_CALL(mLogger, log("INFO", "Server started"));
    EXPECT_CALL(mLogger, log("INFO", "Server stopped"));

    Server svr(&mLogger);
    svr.start();
    svr.stop();
    EXPECT_FALSE(svr.isRunning());
}

TEST_F(ServerTest, connectionTests) {
    MockLogger mlogger;
    testing::internal::CaptureStdout();
    Server svr(&mlogger);
    testing::internal::GetCapturedStdout();

    EXPECT_CALL(mlogger, log("INFO", "Server started"));
    std::thread serverThread(&Server::start, &svr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_TRUE(svr.isRunning());

    // Create a TCP client socket.
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(sock, -1) << "Failed to create socket";

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    int connectStatus = connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    ASSERT_EQ(connectStatus, 0) << "Failed to connect to server";

    EXPECT_CALL(mlogger, log("INFO", "Server stopped"));
    svr.stop();
    EXPECT_FALSE(svr.isRunning());

    // int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    // ASSERT_TRUE(client_socket >= 0);
    serverThread.join();
}
