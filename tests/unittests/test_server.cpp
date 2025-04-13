#include "Server.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>

class MockLogger : public ILogger {
  public:
    MOCK_METHOD(void, log, (const std::string &level, const std::string &msg), (override));
};

// defining a Test Fixture: ServerTest
class ServerTest : public ::testing::Test {};

TEST_F(ServerTest, connectionTests) {
    MockLogger mLogger;
    EXPECT_CALL(mLogger, log("INFO", "Server constructed"));
    EXPECT_CALL(mLogger, log("INFO", "Server is starting..."));
    EXPECT_CALL(mLogger, log("INFO", "Server started"));
    EXPECT_CALL(mLogger, log("INFO", "Server is stopping..."));
    EXPECT_CALL(mLogger, log("INFO", "Server stopped"));
    Server svr(&mLogger);

    std::thread serverThread(&Server::start, &svr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_TRUE(svr.isRunning());

    int i = 0;
    while (i < 7) {
        int clientfd = socket(AF_INET, SOCK_STREAM, 0);
        ASSERT_NE(clientfd, -1) << "Failed to create socket";

        sockaddr_in server_addr = {};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(8080);
        ASSERT_GT(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr), 0);
        ASSERT_EQ(connect(clientfd, (sockaddr *)&server_addr, sizeof(server_addr)), 0);
        i++;
    }
    serverThread.join();
    svr.stop();
}
