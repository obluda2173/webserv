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

class StubLogger : public ILogger {
  public:
    void log(const std::string& level, const std::string& msg) { (void)level, (void)msg; }
};

class ServerTest : public ::testing::TestWithParam<std::vector<int>> {
  protected:
    ILogger* _logger;
    Server _svr;
    std::thread _svrThread;
    int _openFdsBegin;
    std::vector<int> _ports;

    ServerTest() : _logger(new StubLogger()), _svr(_logger), _ports(GetParam()) {}

    void SetUp() override { setupServer(); }

    void TearDown() override { teardownServer(); }

    void setupServer() {
        _openFdsBegin = countOpenFileDescriptors();
        _svrThread = std::thread(&Server::start, &_svr, _ports);
        waitForServerStartup();
    }

    void waitForServerStartup() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        EXPECT_TRUE(_svr.isRunning());
    }

    void teardownServer() {
        _svr.stop();
        EXPECT_FALSE(_svr.isRunning());
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        EXPECT_EQ(_openFdsBegin, countOpenFileDescriptors());
        _svrThread.join();
        delete _logger;
    }
};

// defining a Test Fixture: ServerTest
class ServerWithMockLoggerParametrizedPortTest : public ::testing::TestWithParam<std::vector<int>> {
  protected:
    MockLogger* _mLogger;
    Server _svr;
    std::thread _svrThread;
    int _openFdsBegin;
    std::vector<int> _ports;

    ServerWithMockLoggerParametrizedPortTest() : _mLogger(new MockLogger()), _svr(_mLogger), _ports(GetParam()) {}

    void SetUp() override { setupServer(); }

    void TearDown() override { teardownServer(); }

    void setupServer() {
        _openFdsBegin = countOpenFileDescriptors();
        EXPECT_CALL(*_mLogger, log("INFO", "Server is starting..."));
        EXPECT_CALL(*_mLogger, log("INFO", "Server started"));

        _svrThread = std::thread(&Server::start, &_svr, _ports);
        waitForServerStartup();
    }

    void waitForServerStartup() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        EXPECT_TRUE(_svr.isRunning());
    }

    void teardownServer() {
        EXPECT_CALL(*_mLogger, log("INFO", "Server is stopping..."));
        EXPECT_CALL(*_mLogger, log("INFO", "Server stopped"));

        _svr.stop();
        EXPECT_FALSE(_svr.isRunning());
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        EXPECT_EQ(_openFdsBegin, countOpenFileDescriptors());
        _svrThread.join();
        delete _mLogger;
    }
};

#endif // TEST_FIXTURES_H
