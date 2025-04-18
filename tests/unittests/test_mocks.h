#ifndef TEST_MOCKS_H
#define TEST_MOCKS_H


#include "gmock/gmock.h"
#include "ILogger.h"

class MockLogger : public ILogger {
  public:
    MOCK_METHOD(void, log, (const std::string& level, const std::string& msg), (override));
};

#endif // TEST_MOCKS_H
