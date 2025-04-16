#ifndef TEST_FIXTURES_H
#define TEST_FIXTURES_H

#include "test_main.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <Server.h>
#include <thread>

class MockLogger : public ILogger {
  public:
    MOCK_METHOD(void, log, (const std::string& level, const std::string& msg), (override));
};

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

#endif // TEST_FIXTURES_H
