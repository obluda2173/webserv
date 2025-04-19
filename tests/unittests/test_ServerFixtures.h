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

template <typename LoggerType> class BaseServerTest : public ::testing::TestWithParam<std::vector<std::string>> {
  protected:
    int _openFdsBegin;
    LoggerType* _logger;
    EPollManager* _epollMngr;
    IConnectionHandler* _connHdlr;
    Server* _svr;
    std::thread _svrThread;
    std::vector<std::string> _ports;

  public:
    BaseServerTest()
        : _openFdsBegin(0), _logger(new LoggerType()), _epollMngr(new EPollManager(_logger)),
          _connHdlr(new ConnectionHandler(_logger, _epollMngr)), _svr(nullptr), _ports(GetParam()) {}

    virtual ~BaseServerTest() {
        // Clean up in destructor
        delete _svr;
        delete _connHdlr;
        delete _epollMngr;
        delete _logger;
    }

    void SetUp() override {
        _openFdsBegin = countOpenFileDescriptors();
        _svr = new Server(_logger, _connHdlr, _epollMngr);
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
        EXPECT_EQ(_openFdsBegin, countOpenFileDescriptors());
    }
};

class ServerTestWoMockLogging : public BaseServerTest<StubLogger> {
  public:
    void setupServer() override { BaseServerTest::setupServer(); }

    void teardownServer() override { BaseServerTest::teardownServer(); }
};

// defining a Test Fixture: ServerTest
class ServerWithMockLoggerParametrizedPortTest : public ::testing::TestWithParam<std::vector<std::string>> {
  protected:
    int _openFdsBegin;
    MockLogger* _mLogger;
    EPollManager* _epollMngr;
    IConnectionHandler* _connHdlr;
    Server _svr;
    std::thread _svrThread;
    std::vector<std::string> _ports;

    ServerWithMockLoggerParametrizedPortTest()

        : _openFdsBegin(countOpenFileDescriptors()), _mLogger(new MockLogger()), _epollMngr(new EPollManager(_mLogger)),
          _connHdlr(new ConnectionHandler(_mLogger, _epollMngr)), _svr(_mLogger, _connHdlr, _epollMngr),
          _ports(GetParam()) {}

    void SetUp() override { setupServer(); }

    void TearDown() override { teardownServer(); }

    void setupServer() {
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
        _svrThread.join();
        delete _epollMngr;
        delete _connHdlr;
        delete _mLogger;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        EXPECT_EQ(_openFdsBegin, countOpenFileDescriptors());
    }
};

#endif // TEST_SERVERFIXTURES_H
