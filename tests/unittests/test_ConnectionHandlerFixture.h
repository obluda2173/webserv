#ifndef TEST_CONNECTIONHANDLERTESTFIXTURE_H
#define TEST_CONNECTIONHANDLERTESTFIXTURE_H

#include "ConnectionHandler.h"
#include "EpollIONotifier.h"
#include "HttpResponse.h"
#include "IIONotifier.h"
#include "PingHandler.h"
#include "Router.h"
#include "test_main.h"
#include "test_mocks.h"
#include "test_stubs.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <netdb.h>
#include <sys/socket.h>
#include <utility>
#include <utils.h>

#define CONNECTION_READSIZE 1024

template <typename LoggerType, typename ParamType = int>
class BasConnHdlrTest : public ::testing::TestWithParam<ParamType> {
  protected:
    int _openFdsBegin;
    LoggerType* _logger;
    IIONotifier* _ioNotifier;
    IConnectionHandler* _connHdlr;
    int _serverfd;
    struct addrinfo* _svrAddrInfo;
    int _nbrConns;
    std::vector<std::pair<int, int>> _clientFdsAndConnFds;

  public:
    BasConnHdlrTest() : _openFdsBegin(countOpenFileDescriptors()) {}
    void SetUp() override {
        _openFdsBegin = countOpenFileDescriptors();
        _logger = new LoggerType();
        _ioNotifier = new EpollIONotifier(*_logger);
        setupConnectionHandler();
        setupServer();
        setupClientConnections();
    }

    virtual void setupConnectionHandler() {
        std::map<std::string, IHandler*> hdlrs = {{"GET", new PingHandler()}};
        IRouter* router = new Router(hdlrs);
        router->add("test.com", "", "GET", {});
        _connHdlr = new ConnectionHandler(router, *_logger, *_ioNotifier);
    }

    virtual void setupServer() {
        getAddrInfoHelper(NULL, "8080", AF_INET, &_svrAddrInfo);
        _serverfd = newListeningSocket(_svrAddrInfo, 5);
    }

    virtual void setupClientConnections() {};

    void TearDown() override {
        for (size_t i = 0; i < _clientFdsAndConnFds.size(); i++) {
            close(_clientFdsAndConnFds[i].first);
        }
        freeaddrinfo(_svrAddrInfo);
        close(_serverfd);
        delete _connHdlr;
        delete _ioNotifier;
        delete _logger;
        EXPECT_EQ(_openFdsBegin, countOpenFileDescriptors());
    }
};

class ConnHdlrTestWithMockLoggerIPv6 : public BasConnHdlrTest<MockLogger> {
    void setupServer() override {
        getAddrInfoHelper(NULL, "8080", AF_INET6, &_svrAddrInfo);
        _serverfd = newListeningSocket(_svrAddrInfo, 5);
    }

    virtual void setupClientConnections() override {}
};

struct ParamsVectorRequestsResponses {
    std::vector<std::string> requests;
    std::vector<std::string> wantResponses;
};

class ConnHdlrTestWithOneConnection : public BasConnHdlrTest<StubLogger, ParamsVectorRequestsResponses> {
    virtual void setupClientConnections() override {
        int clientfd;
        int connfd;
        int port = 23456;
        clientfd = newSocket("127.0.0.2", std::to_string(port), AF_INET);
        ASSERT_NE(connect(clientfd, _svrAddrInfo->ai_addr, _svrAddrInfo->ai_addrlen), -1)
            << "connect: " << std::strerror(errno) << std::endl;
        connfd = _connHdlr->handleConnection(_serverfd, READY_TO_READ);
        fcntl(clientfd, F_SETFL, O_NONBLOCK);
        _clientFdsAndConnFds.push_back(std::pair<int, int>{clientfd, connfd});
    }
};

class ConnHdlrTestAsyncMultipleConnectionsReadSizeLimited
    : public BasConnHdlrTest<StubLogger, ParamsVectorRequestsResponses> {
    // special ReadSize
    virtual void setupConnectionHandler() override {
        std::map<std::string, IHandler*> hdlrs = {{"GET", new PingHandler()}};
        IRouter* router = new Router(hdlrs);
        router->add("test.com", "", "GET", {});
        _connHdlr = new ConnectionHandler(router, *_logger, *_ioNotifier);
    }

    virtual void setupClientConnections() override {
        int clientfd;
        int connfd;
        int port = 12345;
        ParamsVectorRequestsResponses params = GetParam();
        int nbrRequests = params.requests.size();
        for (int i = 0; i < nbrRequests; i++) {
            clientfd = newSocket("127.0.0.2", std::to_string(port), AF_INET);
            ASSERT_NE(connect(clientfd, _svrAddrInfo->ai_addr, _svrAddrInfo->ai_addrlen), -1)
                << "connect: " << std::strerror(errno) << std::endl;
            connfd = _connHdlr->handleConnection(_serverfd, READY_TO_READ);
            fcntl(clientfd, F_SETFL, O_NONBLOCK);
            _clientFdsAndConnFds.push_back(std::pair<int, int>{clientfd, connfd});
            port++;
        }
    }
};

class ConnHdlrTestWithIntegerAsParameter : public BasConnHdlrTest<StubLogger> {
    virtual void setupClientConnections() override {
        int clientfd;
        int connfd;
        int port = 23456;
        clientfd = newSocket("127.0.0.2", std::to_string(port), AF_INET);
        ASSERT_NE(connect(clientfd, _svrAddrInfo->ai_addr, _svrAddrInfo->ai_addrlen), -1)
            << "connect: " << std::strerror(errno) << std::endl;
        connfd = _connHdlr->handleConnection(_serverfd, READY_TO_READ);
        fcntl(clientfd, F_SETFL, O_NONBLOCK);
        _clientFdsAndConnFds.push_back(std::pair<int, int>{clientfd, connfd});
    }
};

class BigRespBodyGetHandler : public IHandler {

  public:
    std::string _body;
    BigRespBodyGetHandler(std::string body) : _body(body) {}
    virtual void handle(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
        (void)req;
        (void)cfg;
        HttpResponse& resp = conn->_response;
        resp.statusCode = 200;
        resp.statusMessage = "OK";
        resp.contentLength = _body.length();
        resp.body = new StringBodyProvider(_body);
        resp.version = "HTTP/1.1";
        conn->setState(Connection::SendResponse);
        return;
    };
};

class ConnHdlrTestWithBigResponseBody : public BasConnHdlrTest<StubLogger, int> {
  public:
    std::string _body;
    virtual void setupConnectionHandler() override {
        _body = getRandomString(10000);
        std::map<std::string, IHandler*> hdlrs = {{"GET", new BigRespBodyGetHandler(_body)}};
        IRouter* router = new Router(hdlrs);
        router->add("test.com", "", "GET", {});
        _connHdlr = new ConnectionHandler(router, *_logger, *_ioNotifier);
    }

    virtual void setupClientConnections() override {
        int clientfd;
        int connfd;
        int port = 23456;
        clientfd = newSocket("127.0.0.2", std::to_string(port), AF_INET);
        ASSERT_NE(connect(clientfd, _svrAddrInfo->ai_addr, _svrAddrInfo->ai_addrlen), -1)
            << "connect: " << std::strerror(errno) << std::endl;
        connfd = _connHdlr->handleConnection(_serverfd, READY_TO_READ);
        fcntl(clientfd, F_SETFL, O_NONBLOCK);
        _clientFdsAndConnFds.push_back(std::pair<int, int>{clientfd, connfd});
    }
};

class StubUploadHandler : public IHandler {
  public:
    std::string _uploaded;
    virtual void handle(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
        (void)req;
        (void)cfg;
        _uploaded += conn->getReadBuf();
        HttpResponse& resp = conn->_response;
        resp.statusCode = 200;
        resp.statusMessage = "OK";
        resp.version = "HTTP/1.1";
        conn->setState(Connection::SendResponse);
    }
};

class ConnHdlrTestUpload : public BasConnHdlrTest<StubLogger, int> {
  public:
    StubUploadHandler* _uploadHdlr;
    virtual void setupConnectionHandler() override {
        _uploadHdlr = new StubUploadHandler();
        std::map<std::string, IHandler*> hdlrs = {{"POST", _uploadHdlr}};
        IRouter* router = new Router(hdlrs);
        router->add("test.com", "", "POST", {});
        _connHdlr = new ConnectionHandler(router, *_logger, *_ioNotifier);
    }

    virtual void setupClientConnections() override {
        int clientfd;
        int connfd;
        int port = 23456;
        clientfd = newSocket("127.0.0.2", std::to_string(port), AF_INET);
        ASSERT_NE(connect(clientfd, _svrAddrInfo->ai_addr, _svrAddrInfo->ai_addrlen), -1)
            << "connect: " << std::strerror(errno) << std::endl;
        connfd = _connHdlr->handleConnection(_serverfd, READY_TO_READ);
        fcntl(clientfd, F_SETFL, O_NONBLOCK);
        _clientFdsAndConnFds.push_back(std::pair<int, int>{clientfd, connfd});
    }
};

#endif // TEST_CONNECTIONHANDLERTESTFIXTURE_H
