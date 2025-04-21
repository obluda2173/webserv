#ifndef TEST_SERVERFIXTURES_H
#define TEST_SERVERFIXTURES_H

#include "ConnectionHandler.h"
#include "EpollIONotifier.h"
#include "IConnectionHandler.h"
#include "IIONotifier.h"
#include "ServerBuilder.h"
#include "test_main.h"
#include "test_mocks.h"
#include "test_stubs.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <thread>

template <typename LoggerType> class BaseServerTest : public ::testing::TestWithParam<std::vector<std::string>> {
  protected:
    int _openFdsBegin;
    LoggerType* _logger;
    Server* _svr;
    std::thread _svrThread;
    std::vector<std::string> _ports;

  public:
    BaseServerTest() : _openFdsBegin(countOpenFileDescriptors()), _ports(GetParam()) {}

    virtual ~BaseServerTest() {} // only for childs

    void SetUp() override {
        _logger = new LoggerType();
        IIONotifier* ioNotifier = new EpollIONotifier(*_logger);
        IConnectionHandler* connHdlr = new ConnectionHandler(*_logger, *ioNotifier);
        _svr = ServerBuilder().setLogger(_logger).setIONotifier(ioNotifier).setConnHdlr(connHdlr).build();
        setupServer();
    }

    void TearDown() override { teardownServer(); }

    void waitForServerStartup() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        EXPECT_TRUE(_svr->isRunning());
    }

    virtual void setupServer() {
        _svrThread = std::thread(&Server::start, _svr, _ports);
        waitForServerStartup();
    }

    virtual void teardownServer() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        _svr->stop();
        EXPECT_FALSE(_svr->isRunning());
        if (_svrThread.joinable()) {
            _svrThread.join();
        }
        delete _svr;
        EXPECT_EQ(_openFdsBegin, countOpenFileDescriptors());
    }
};

class ServerTestWoMockLogging : public BaseServerTest<StubLogger> {
  public:
    void setupServer() override { BaseServerTest::setupServer(); }

    void teardownServer() override { BaseServerTest::teardownServer(); }
};

class ServerWithMockLoggerParametrizedPortTest : public BaseServerTest<MockLogger> {
  public:
    void setupServer() override {
        EXPECT_CALL(*_logger, log("INFO", "Server is starting..."));
        EXPECT_CALL(*_logger, log("INFO", "Server started"));

        _svrThread = std::thread(&Server::start, _svr, _ports);
        waitForServerStartup();
    }
    void teardownServer() override {
        EXPECT_CALL(*_logger, log("INFO", "Server is stopping..."));
        EXPECT_CALL(*_logger, log("INFO", "Server stopped"));
        _svr->stop();
        EXPECT_FALSE(_svr->isRunning());
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        _svrThread.join();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        delete _svr;
        EXPECT_EQ(_openFdsBegin, countOpenFileDescriptors());
    }
};

#endif // TEST_SERVERFIXTURES_H
