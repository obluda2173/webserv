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
#include <utility>
#include <utils.h>

template <typename LoggerType, typename ParamType = int>
class BaseConnectionHandlerTest : public ::testing::TestWithParam<ParamType> {
  protected:
    int _openFdsBegin;
    LoggerType* _logger;
    IIONotifier* _ioNotifier;
    IConnectionHandler* _connHdlr;
    int _serverfd;
    struct addrinfo* _svrAddrInfo;
    int _nbrConns;
    std::vector<std::pair<int, int>> _clientfdsAndConns;

  public:
    BaseConnectionHandlerTest() : _openFdsBegin(countOpenFileDescriptors()) {}
    void SetUp() override {
        _openFdsBegin = countOpenFileDescriptors();
        _logger = new LoggerType();
        _ioNotifier = new EpollIONotifier(*_logger);
        _connHdlr = new ConnectionHandler(*_logger, *_ioNotifier);
        setupServer();
        setupClientConnections();
    }

    virtual void setupServer() {
        getAddrInfoHelper(NULL, "8080", AF_INET, &_svrAddrInfo);
        _serverfd = newListeningSocket(_svrAddrInfo, 5);
    }

    virtual void setupClientConnections() {};

    void TearDown() override {
        for (size_t i = 0; i < _clientfdsAndConns.size(); i++) {
            close(_clientfdsAndConns[i].first);
        }
        freeaddrinfo(_svrAddrInfo);
        close(_serverfd);
        delete _connHdlr;
        delete _ioNotifier;
        delete _logger;
        EXPECT_EQ(_openFdsBegin, countOpenFileDescriptors());
    }
};

class ConnectionHdlrTestBase : public BaseConnectionHandlerTest<StubLogger, int> {};

class ConnectionHdlrTestWithMockLoggerIPv6 : public BaseConnectionHandlerTest<MockLogger> {
    void setupServer() override {
        getAddrInfoHelper(NULL, "8080", AF_INET6, &_svrAddrInfo);
        _serverfd = newListeningSocket(_svrAddrInfo, 5);
    }

    virtual void setupClientConnections() override {}
};

struct ParamsConnectionHdlrTestAsync {
    std::vector<std::string> requests;
    std::vector<std::string> wantResponses;
};

class ConnectionHdlrTestAsync : public BaseConnectionHandlerTest<StubLogger, ParamsConnectionHdlrTestAsync> {
    virtual void setupClientConnections() {
        int clientfd;
        int conn;
        int port = 12345;
        ParamsConnectionHdlrTestAsync params = GetParam();
        int nbrRequests = params.requests.size();
        for (int i = 0; i < nbrRequests; i++) {
            clientfd = newSocket("127.0.0.2", std::to_string(port), AF_INET);
            ASSERT_NE(connect(clientfd, _svrAddrInfo->ai_addr, _svrAddrInfo->ai_addrlen), -1)
                << "connect: " << std::strerror(errno) << std::endl;
            conn = _connHdlr->handleConnection(_serverfd, READY_TO_READ);
            fcntl(clientfd, F_SETFL, O_NONBLOCK);
            _clientfdsAndConns.push_back(std::pair<int, int>{clientfd, conn});
            port++;
        }
    }
};

///////////////////////
// parametrized test //
///////////////////////

//////////////////////////////////////////
// with an int as parameter (batchSize) //
//////////////////////////////////////////

template <typename LoggerType, typename ParamType = int>
class BaseConnectionHandlerTestWithParam : public ::testing::TestWithParam<ParamType> {
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
    BaseConnectionHandlerTestWithParam() : _openFdsBegin(countOpenFileDescriptors()) {}
    void SetUp() override {
        _openFdsBegin = countOpenFileDescriptors();
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
        EXPECT_EQ(_openFdsBegin, countOpenFileDescriptors());
    }
};

class ConnectionHdlrTestWithParamInt : public BaseConnectionHandlerTestWithParam<StubLogger> {};

struct reqRespParam {
    std::string request;
    std::string wantResponse;
};
class ConnectionHdlrTestWithParamReqResp : public BaseConnectionHandlerTestWithParam<StubLogger, reqRespParam> {};

#endif // TEST_CONNECTIONHANDLERTESTFIXTURE_H
