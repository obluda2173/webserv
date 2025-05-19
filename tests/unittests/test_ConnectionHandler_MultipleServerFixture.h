#ifndef TEST_CONNECTIONHANDLER_MULTIPLESERVERFIXTURE_H
#define TEST_CONNECTIONHANDLER_MULTIPLESERVERFIXTURE_H

#include "IIONotifier.h"
#include "PingHandler.h"
#include "EpollIONotifier.h"
#include "ConnectionHandler.h"
#include "Router.h"
#include "test_main.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <netdb.h>
#include <sys/socket.h>
#include <utility>
#include <utils.h>
#include "test_stubs.h"

class ConnHdlrTestMultipleRouter : public ::testing::Test {
  protected:
    int _openFdsBegin;
    StubLogger* _logger;
    IIONotifier* _ioNotifier;
    IConnectionHandler* _connHdlr;
    int _serverfd;
    struct addrinfo* _svrAddrInfo;
    int _nbrConns;
    std::vector< std::pair< int, int > > _clientFdsAndConnFds;

  public:
    ConnHdlrTestMultipleRouter() : _openFdsBegin(countOpenFileDescriptors()) {}
    void SetUp() override {
        _openFdsBegin = countOpenFileDescriptors();
        _logger = new StubLogger();
        _ioNotifier = new EpollIONotifier(*_logger);
        setupConnectionHandler();
        setupServer();
        setupClientConnections();
    }

    virtual void setupConnectionHandler() {
        std::map< std::string, IHandler* > hdlrs = {{"GET", new PingHandler()}};
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




#endif // TEST_CONNECTIONHANDLER_MULTIPLESERVERFIXTURE_H
