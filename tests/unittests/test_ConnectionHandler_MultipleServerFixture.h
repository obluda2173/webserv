#ifndef TEST_CONNECTIONHANDLER_MULTIPLESERVERFIXTURE_H
#define TEST_CONNECTIONHANDLER_MULTIPLESERVERFIXTURE_H

#include "ConnectionHandler.h"
#include "EpollIONotifier.h"
#include "IIONotifier.h"
#include "PingHandler.h"
#include "Router.h"
#include "test_main.h"
#include "test_stubs.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <netdb.h>
#include <sys/socket.h>
#include <utils.h>

const static int backlog = 5;

class ConnHdlrTestMultipleRouter : public ::testing::Test {
  protected:
    int _openFdsBegin;
    StubLogger* _logger;
    IIONotifier* _ioNotifier;
    IConnectionHandler* _connHdlr;
    struct addrinfo* _svrAddrInfo8080;
    struct addrinfo* _svrAddrInfo8081;
    int _serverfd8080;
    int _serverfd8081;

    int _clientfd8080;
    int _clientfd8081;
    int _connfd8080;
    int _connfd8081;
    int _nbrConns;

  public:
    ConnHdlrTestMultipleRouter() : _openFdsBegin(countOpenFileDescriptors()) {}
    void SetUp() override {
        _openFdsBegin = countOpenFileDescriptors();
        _logger = new StubLogger();
        _ioNotifier = new EpollIONotifier(*_logger);
        setupConnectionHandler();
        setupServer();
        setupClientConnections8080();
        setupClientConnections8081();
    }

    virtual void setupConnectionHandler() {
        std::map< int, IRouter* > routers;

        std::map< std::string, IHandler* > hdlrs = {{"GET", new PingHandler()}};
        IRouter* router = new Router(hdlrs);
        router->add("test.com", "", "GET", {});
        routers[8080] = router;

        hdlrs = {{"GET", new PingHandler2()}};
        router = new Router(hdlrs);
        router->add("test.com", "", "GET", {});
        routers[8081] = router;

        _connHdlr = new ConnectionHandler(routers, *_logger, *_ioNotifier);
    }

    virtual void setupServer() {
        getAddrInfoHelper(NULL, "8080", AF_INET, &_svrAddrInfo8080);
        _serverfd8080 = newListeningSocket(_svrAddrInfo8080, backlog);

        getAddrInfoHelper(NULL, "8081", AF_INET, &_svrAddrInfo8081);
        _serverfd8081 = newListeningSocket(_svrAddrInfo8081, backlog);
    }

    void TearDown() override {
        freeaddrinfo(_svrAddrInfo8080);
        freeaddrinfo(_svrAddrInfo8081);
        close(_serverfd8080);
        close(_serverfd8081);
        close(_clientfd8080);
        close(_clientfd8081);
        close(_connfd8080);
        close(_connfd8081);
        delete _connHdlr;
        delete _ioNotifier;
        delete _logger;
        EXPECT_EQ(_openFdsBegin, countOpenFileDescriptors());
    }

    virtual void setupClientConnections8080() {
        int port = 23456;
        _clientfd8080 = newSocket("127.0.0.2", std::to_string(port), AF_INET);
        ASSERT_NE(connect(_clientfd8080, _svrAddrInfo8080->ai_addr, _svrAddrInfo8080->ai_addrlen), -1)
            << "connect: " << std::strerror(errno) << std::endl;
        _connfd8080 = _connHdlr->handleConnection(_serverfd8080, READY_TO_READ);
        fcntl(_clientfd8080, F_SETFL, O_NONBLOCK);
    }

    virtual void setupClientConnections8081() {
        int port = 12345;
        _clientfd8081 = newSocket("127.0.0.2", std::to_string(port), AF_INET);
        ASSERT_NE(connect(_clientfd8081, _svrAddrInfo8081->ai_addr, _svrAddrInfo8081->ai_addrlen), -1)
            << "connect: " << std::strerror(errno) << std::endl;
        _connfd8081 = _connHdlr->handleConnection(_serverfd8081, READY_TO_READ);
        fcntl(_clientfd8081, F_SETFL, O_NONBLOCK);
    }
};

#endif // TEST_CONNECTIONHANDLER_MULTIPLESERVERFIXTURE_H
