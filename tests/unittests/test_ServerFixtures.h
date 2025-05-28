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
#include <set>
#include <thread>

template < typename LoggerType >
class BaseServerTest : public ::testing::TestWithParam< std::set< std::pair< std::string, std::string > > > {
  protected:
    int _openFdsBegin;
    LoggerType* _logger;
    Server* _svr;
    std::thread _svrThread;
    std::set< std::pair< std::string, std::string > > _addrPorts;

  public:
    BaseServerTest() : _openFdsBegin(countOpenFileDescriptors()), _addrPorts(GetParam()) {}

    virtual ~BaseServerTest() {} // only for childs

    void SetUp() override {
        _logger = new LoggerType();
        IIONotifier* ioNotifier = new EpollIONotifier(*_logger);

        // router will be owned by Connection Handler
        std::map< std::string, IHandler* > hdlrs = {{}};
        IRouter* router = new Router(hdlrs);

        std::map< std::string, IRouter* > routers;
        routers["0.0.0.0:8080"] = router;
        IConnectionHandler* connHdlr = new ConnectionHandler(routers, *_logger, *ioNotifier);

        _svr = ServerBuilder().setLogger(_logger).setIONotifier(ioNotifier).setConnHdlr(connHdlr).build();
        setupServer();
    }

    void TearDown() override { teardownServer(); }

    void waitForServerStartup() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        EXPECT_TRUE(_svr->isRunning());
    }

    virtual void setupServer() {
        _svrThread = std::thread(&Server::start, _svr, _addrPorts, nullptr);
        waitForServerStartup();
    }

    virtual void teardownServer() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        _svr->stop();
        EXPECT_FALSE(_svr->isRunning());
        if (_svrThread.joinable()) {
            _svrThread.join();
        }
        delete _svr;
        EXPECT_EQ(_openFdsBegin, countOpenFileDescriptors());
    }
};

class ServerTestWoMockLogging : public BaseServerTest< StubLogger > {
  public:
    void setupServer() override { BaseServerTest::setupServer(); }
    void teardownServer() override { BaseServerTest::teardownServer(); }
};

class ServerWithMockLoggerParametrizedPortTest : public BaseServerTest< testing::NiceMock< MockLogger > > {
  public:
    void setupServer() override {
        using ::testing::_;
        EXPECT_CALL(*_logger, log(_, _)).Times(testing::AnyNumber());
        EXPECT_CALL(*_logger, log("INFO", "Server is starting...")).Times(1);
        EXPECT_CALL(*_logger, log("INFO", "Server started")).Times(1);

        _svrThread = std::thread(&Server::start, _svr, _addrPorts, nullptr);
        waitForServerStartup();
    }
    void teardownServer() override {
        EXPECT_CALL(*_logger, log("INFO", "Server is stopping...")).Times(1);
        EXPECT_CALL(*_logger, log("INFO", "Server stopped")).Times(1);
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
