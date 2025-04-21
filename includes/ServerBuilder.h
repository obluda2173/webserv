#ifndef SERVERBUILDER_H
#define SERVERBUILDER_H

#include "IConnectionHandler.h"
#include "IIONotifier.h"
#include "Server.h"

class ServerBuilder {
  private:
    ILogger* _logger;
    IConnectionHandler* _connHdlr;
    IIONotifier* _ioNotifer;

  public:
    Server* build(void) { return new Server(_logger, _connHdlr, _ioNotifer); }
    ServerBuilder& setLogger(ILogger* logger) {
        _logger = logger;
        return *this;
    }
    ServerBuilder& setIONotifier(IIONotifier* ioNotifier) {
        _ioNotifer = ioNotifier;
        return *this;
    }
    ServerBuilder& setConnHdlr(IConnectionHandler* connHdlr) {
        _connHdlr = connHdlr;
        return *this;
    }
};

#endif // SERVERBUILDER_H
