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
    int _clientfd;
    int _conn;

  public:
    BaseConnectionHandlerTest() : _openFdsBegin(countOpenFileDescriptors()) {}
    void SetUp() override {
        _logger = new LoggerType();
        _ioNotifier = new EpollIONotifier(*_logger);
        _connHdlr = new ConnectionHandler(*_logger, *_ioNotifier);
        setupServer();
        setupConnection();
    }

    virtual void setupServer() {
        getAddrInfoHelper(NULL, "8080", AF_INET, &_svrAddrInfo);
        _serverfd = newListeningSocket(_svrAddrInfo, 5);
    }

    virtual void setupConnection() {
        _clientfd = newSocket("127.0.0.2", "12345", AF_INET);
        ASSERT_NE(connect(_clientfd, _svrAddrInfo->ai_addr, _svrAddrInfo->ai_addrlen), -1)
            << "connect: " << std::strerror(errno) << std::endl;
        _conn = _connHdlr->handleConnection(_serverfd, READY_TO_READ);
        fcntl(_clientfd, F_SETFL, O_NONBLOCK);
    }

    void TearDown() override {
        close(_clientfd);
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

    virtual void setupConnection() override {}
};
class ConnectionHdlrTest : public BaseConnectionHandlerTest<StubLogger> {};

#endif // TEST_CONNECTIONHANDLERTESTFIXTURE_H
