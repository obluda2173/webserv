#include "Server.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <arpa/inet.h>
#include <fcntl.h>
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

TEST_F(ServerTest, connectionTest) {
    MockLogger mLogger;
    EXPECT_CALL(mLogger, log("INFO", "Server constructed"));
    Server svr(&mLogger);

    EXPECT_CALL(mLogger, log("INFO", "Server is starting..."));
    EXPECT_CALL(mLogger, log("INFO", "Server started"));
    std::thread serverThread(&Server::start, &svr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_TRUE(svr.isRunning());

    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(clientfd, -1) << "Failed to create socket";

    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    ASSERT_GT(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr), 0);

    EXPECT_CALL(mLogger, log("INFO", "Accepted connection"));
    ASSERT_EQ(connect(clientfd, (sockaddr *)&server_addr, sizeof(server_addr)), 0);

    struct sockaddr_in local_address;
    socklen_t address_length = sizeof(local_address);
    ASSERT_NE(getsockname(clientfd, (struct sockaddr *)&local_address, &address_length), -1);
    char local_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &local_address.sin_addr, local_ip, sizeof(local_ip));
    int local_port = ntohs(local_address.sin_port);
    std::cout << "Local IP: " << local_ip << ", Local Port: " << local_port << std::endl;

    serverThread.join();

    EXPECT_CALL(mLogger, log("INFO", "Server is stopping..."));
    EXPECT_CALL(mLogger, log("INFO", "Server stopped"));
    svr.stop();
}
