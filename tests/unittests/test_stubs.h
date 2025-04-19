#ifndef TEST_STUBS_H
#define TEST_STUBS_H

#include "ILogger.h"

class StubLogger : public ILogger {
  public:
    void log(const std::string&, const std::string&) {}
};

#endif // TEST_STUBS_H
