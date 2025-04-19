// #ifndef TEST_LISTENERFIXTURES_H
// #define TEST_LISTENERFIXTURES_H

// #include "ConnectionHandler.h"
// #include "EPollManager.h"
// #include "IConnectionHandler.h"
// #include "Listener.h"
// #include "test_main.h"
// #include "test_mocks.h"
// #include <gtest/gtest.h>
// #include <thread>

// class ListenerTestWithMockLogging : public ::testing::TestWithParam<std::vector<std::string>> {
//   protected:
//     MockLogger* _mLogger;
//     EPollManager* _epollMngr;
//     IConnectionHandler* _connHdlr;
//     Listener* _listener;
//     std::thread _listenerThread;
//     int _openFdsBegin;
//     std::vector<std::string> _ports;

//   public:
//     ListenerTestWithMockLogging()
//         : _mLogger(new MockLogger), _epollMngr(new EPollManager(_mLogger)),
//           _connHdlr(new ConnectionHandler(_mLogger, _epollMngr)),
//           _listener(new Listener(_mLogger, _connHdlr, _epollMngr)), _ports(GetParam()) {}

//     void SetUp() override { setupListener(); }

//     void setupListener() {
//         _openFdsBegin = countOpenFileDescriptors();
//         for (size_t i = 0; i < _ports.size(); i++)
//             _listener->add(_ports[i]);
//         _listenerThread = std::thread(&Listener::listen, &_listener, _ports);
//     }
// };

// #endif // TEST_LISTENERFIXTURES_H
