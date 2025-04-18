#ifndef TEST_STUBS_H
#define TEST_STUBS_H

#include "ILogger.h"

class StubLogger : public ILogger {
  public:
    void log(const std::string& level, const std::string& msg) { (void)level, (void)msg; }
};


#endif // TEST_STUBS_H
