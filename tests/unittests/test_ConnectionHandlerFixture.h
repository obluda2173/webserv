#ifndef TEST_CONNECTIONHANDLERTESTFIXTURE_H
#define TEST_CONNECTIONHANDLERTESTFIXTURE_H

#include "ConnectionHandler.h"
#include "EpollIONotifier.h"
#include "IIONotifier.h"
#include "test_main.h"
#include "test_mocks.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

template <typename LoggerType> class BaseConnectionHandlerTest : public ::testing::Test {
  protected:
    int _openFdsBegin;
    LoggerType* _logger;
    IIONotifier* _ioNotifier;
    IConnectionHandler* _connHdlr;

  public:
    BaseConnectionHandlerTest() : _openFdsBegin(countOpenFileDescriptors()) {}
    void SetUp() override {
        _logger = new LoggerType();
        _ioNotifier = new EpollIONotifier(*_logger);
        _connHdlr = new ConnectionHandler(*_logger, *_ioNotifier);
    }

    void TearDown() override {
        delete _connHdlr;
        delete _ioNotifier;
        delete _logger;
    }
};

class ConnectionHdlrTestWithMockLogger : public BaseConnectionHandlerTest<MockLogger> {};

#endif // TEST_CONNECTIONHANDLERTESTFIXTURE_H
