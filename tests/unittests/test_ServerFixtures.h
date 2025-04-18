#ifndef TEST_SERVERFIXTURES_H
#define TEST_SERVERFIXTURES_H

#include "ConnectionHandler.h"
#include "EPollManager.h"
#include "test_main.h"
#include "test_mocks.h"
#include "test_stubs.h"
#include "gtest/gtest.h"
#include <Server.h>
#include <thread>

class ServerTest : public ::testing::TestWithParam<std::vector<std::string>> {
  protected:
    ILogger* _logger;
    EPollManager* _epollMngr;
    IConnectionHandler* _connHdlr;
    Server _svr;
    std::thread _svrThread;
    int _openFdsBegin;
    std::vector<std::string> _ports;

    ServerTest()
        : _logger(new StubLogger()), _epollMngr(new EPollManager(_logger)),
          _connHdlr(new ConnectionHandler(_logger, _epollMngr)), _svr(_logger, _connHdlr, _epollMngr),
          _ports(GetParam()) {}

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
        delete _epollMngr;
        delete _connHdlr;
    }
};

// defining a Test Fixture: ServerTest
class ServerWithMockLoggerParametrizedPortTest : public ::testing::TestWithParam<std::vector<std::string>> {
  protected:
    MockLogger* _mLogger;
    EPollManager* _epollMngr;
    IConnectionHandler* _connHdlr;
    Server _svr;
    std::thread _svrThread;
    int _openFdsBegin;
    std::vector<std::string> _ports;

    ServerWithMockLoggerParametrizedPortTest()

        : _mLogger(new MockLogger()), _epollMngr(new EPollManager(_mLogger)),
          _connHdlr(new ConnectionHandler(_mLogger, _epollMngr)), _svr(_mLogger, _connHdlr, _epollMngr),
          _ports(GetParam()) {}

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
        delete _epollMngr;
        delete _connHdlr;
    }
};

#endif // TEST_SERVERFIXTURES_H
