#include "test_main.h"
#include "Server.h"

class MockLogger : public ILogger {
  public:
    MOCK_METHOD(void, log, (const std::string &level, const std::string &msg), (override));
};

// defining a Test Fixture: ServerTest
class ServerTest : public ::testing::Test {};

TEST_F(ServerTest, connectionTest) {
    MockLogger mLogger;
    EXPECT_CALL(mLogger, log("INFO", "Server constructed"));
    Server svr(&mLogger);

    EXPECT_CALL(mLogger, log("INFO", "Server is starting..."));
    EXPECT_CALL(mLogger, log("INFO", "Server started"));
    std::thread serverThread(&Server::start, &svr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_TRUE(svr.isRunning());

    int clientfd = getClientSocket("127.0.0.2", 8081);

    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    ASSERT_GT(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr), 0);

    EXPECT_CALL(mLogger, log("INFO", "Connection accepted from IP: 127.0.0.2, Port: 8081"));
    ASSERT_EQ(connect(clientfd, (sockaddr *)&server_addr, sizeof(server_addr)), 0);

    serverThread.join();
    EXPECT_CALL(mLogger, log("INFO", "Server is stopping..."));
    EXPECT_CALL(mLogger, log("INFO", "Server stopped"));
    svr.stop();
}

