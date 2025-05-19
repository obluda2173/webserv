#ifndef TEST_STUBS_H
#define TEST_STUBS_H

#include "HttpRequest.h"
#include "IClock.h"
#include "IHandler.h"
#include "ILogger.h"

class StubLogger : public ILogger {
  public:
    void log(const std::string&, const std::string&) {}
};

class StubHandler : public IHandler {
  public:
    StubHandler(std::string type) : type(type) {}
    ~StubHandler() {}
    std::string type;
    virtual void handle(Connection*, const HttpRequest&, const RouteConfig&) {};
};

class StubClock : public IClock {
  private:
    long _sec;
    long _usec;

  public:
    StubClock(long sec = 0, long usec = 0) : _sec(sec), _usec(usec) {}
    timeval now() const { return timeval{_sec, _usec}; }
    void advance(long milli) {
        _usec += milli * 1000;
        _sec += _usec / 1000000;
        _usec = _usec % 1000000;
    }
};
#endif // TEST_STUBS_H
