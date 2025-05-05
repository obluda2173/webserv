#ifndef TEST_STUBS_H
#define TEST_STUBS_H

#include "HttpRequest.h"
#include "ILogger.h"
#include "Router.h"

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

#endif // TEST_STUBS_H
