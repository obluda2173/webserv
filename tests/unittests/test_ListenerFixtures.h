#ifndef TEST_LISTENERFIXTURES_H
#define TEST_LISTENERFIXTURES_H

#include "ConnectionHandler.h"
#include "EpollIONotifier.h"
#include "Listener.h"
#include "test_main.h"
#include "test_mocks.h"
#include "test_stubs.h"
#include "utils.h"
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <thread>

template < typename LoggerType >
class BaseListenerTest : public ::testing::TestWithParam< std::vector< int > > {
  protected:
    int _openFdsBegin;
    LoggerType* _logger;
    Listener* _listener;
    int _backlog;
    std::thread _listenerThread;
    std::vector< int > _ports;

  public:
    BaseListenerTest() : _openFdsBegin(countOpenFileDescriptors()), _backlog(5), _ports(GetParam()) {}

    void SetUp() override { setupListener(); }

    void TearDown() override { tearDownListener(); }

    void setupListener() {
        _logger = new LoggerType();
        EpollIONotifier* ioNotifier = new EpollIONotifier(*_logger);

        // Router will be owned by ConnectionHandler
        std::map< std::string, IHandler* > hdlrs = {{}};
        IRouter* router = new Router(hdlrs);
        std::map< std::string, IRouter* > routers;
        routers["0.0.0.0:8080"] = router;
        ConnectionHandler* connHdlr = new ConnectionHandler(routers, *_logger, *ioNotifier);

        _listener = new Listener(*_logger, connHdlr, ioNotifier);

        for (size_t i = 0; i < _ports.size(); i++) {
            struct addrinfo* svrAddrInfo;
            getAddrInfoHelper(NULL, std::to_string(_ports[i]).c_str(), AF_INET, &svrAddrInfo);
            int portfd = newListeningSocket(svrAddrInfo, 5);
            freeaddrinfo(svrAddrInfo);
            _listener->add(portfd);
        }

        _listenerThread = std::thread(&Listener::listen, _listener);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    void tearDownListener() {
        _listener->stop();
        if (_listenerThread.joinable())
            _listenerThread.join();
        // Add a small delay to ensure all pending operations complete
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        delete _listener;
        delete _logger;

        ASSERT_EQ(_openFdsBegin, countOpenFileDescriptors());
    }
};

// class ListenerTestWithMockLogging : public BaseListenerTest< MockLogger > {};
class ListenerTestWithMockLogging : public BaseListenerTest< testing::NiceMock< MockLogger > > {};
class ListenerTestWoMockLogging : public BaseListenerTest< StubLogger > {};
#endif // TEST_LISTENERFIXTURES_H
