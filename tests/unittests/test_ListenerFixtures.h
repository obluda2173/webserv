#ifndef TEST_LISTENERFIXTURES_H
#define TEST_LISTENERFIXTURES_H

#include "ConnectionHandler.h"
#include "EPollManager.h"
#include "IConnectionHandler.h"
#include "Listener.h"
#include "test_main.h"
#include "test_mocks.h"
#include "utils.h"
#include <gtest/gtest.h>
#include <thread>

class ListenerTestWithMockLogging : public ::testing::TestWithParam<std::vector<int>> {
  protected:
    int _openFdsBegin;
    MockLogger* _mLogger;
    EPollManager* _epollMngr;
    IConnectionHandler* _connHdlr;
    Listener* _listener;
    std::thread _listenerThread;
    std::vector<int> _ports;
    std::vector<int> _portfds;

  public:
    ListenerTestWithMockLogging()
        : _openFdsBegin(countOpenFileDescriptors()), _mLogger(new MockLogger), _epollMngr(new EPollManager(_mLogger)),
          _connHdlr(new ConnectionHandler(_mLogger, _epollMngr)),
          _listener(new Listener(_mLogger, _connHdlr, _epollMngr)), _ports(GetParam()) {}

    void SetUp() override { setupListener(); }

    void TearDown() override { tearDownListener(); }
    void setupListener() {
        ;
        for (size_t i = 0; i < _ports.size(); i++) {
            int portfd = newListeningSocket1(NULL, std::to_string(_ports[i]).c_str());
            _listener->add(portfd);
            _portfds.push_back(portfd);
        }
        _listenerThread = std::thread(&Listener::listen, _listener);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    void tearDownListener() {
        _listener->stop();
        _listenerThread.join();
        delete _connHdlr;
        delete _epollMngr;
        delete _listener;
        delete _mLogger;
        for (size_t i = 0; i < _portfds.size(); i++)
            ASSERT_NE(close(_portfds[i]), -1);
        ASSERT_EQ(_openFdsBegin, countOpenFileDescriptors());
    }
};

#endif // TEST_LISTENERFIXTURES_H
