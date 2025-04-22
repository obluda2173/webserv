#ifndef TEST_CONNECTIONHANDLERTESTFIXTURE_H
#define TEST_CONNECTIONHANDLERTESTFIXTURE_H

#include "ConnectionHandler.h"
#include "EpollIONotifier.h"
#include "IIONotifier.h"
#include "test_main.h"
#include "test_mocks.h"
#include "test_stubs.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <netdb.h>
#include <sys/socket.h>
#include <utils.h>

template <typename LoggerType> class BaseConnectionHandlerTest : public ::testing::Test {
  protected:
    int _openFdsBegin;
    LoggerType* _logger;
    IIONotifier* _ioNotifier;
    IConnectionHandler* _connHdlr;
    int _serverfd;
    struct addrinfo* _svrAddrInfo;

  public:
    BaseConnectionHandlerTest() : _openFdsBegin(countOpenFileDescriptors()) {}
    void SetUp() override {
        _logger = new LoggerType();
        _ioNotifier = new EpollIONotifier(*_logger);
        _connHdlr = new ConnectionHandler(*_logger, *_ioNotifier);
        setupServer();
    }

    virtual void setupServer() {
        getAddrInfoHelper(NULL, "8080", AF_INET, &_svrAddrInfo);
        _serverfd = newListeningSocket(_svrAddrInfo, 5);
    }

    void TearDown() override {
        freeaddrinfo(_svrAddrInfo);
        close(_serverfd);
        delete _connHdlr;
        delete _ioNotifier;
        delete _logger;
    }
};

class ConnectionHdlrTestWithMockLoggerIPv6 : public BaseConnectionHandlerTest<MockLogger> {
    void setupServer() override {
        getAddrInfoHelper(NULL, "8080", AF_INET6, &_svrAddrInfo);
        _serverfd = newListeningSocket(_svrAddrInfo, 5);
    }
};
class ConnectionHdlrTest : public BaseConnectionHandlerTest<StubLogger> {};

#endif // TEST_CONNECTIONHANDLERTESTFIXTURE_H
