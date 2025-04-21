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
    Server* build(void);
    ServerBuilder& setLogger(ILogger* logger);
    ServerBuilder& setIONotifier(IIONotifier* ioNotifier);
    ServerBuilder& setConnHdlr(IConnectionHandler* connHdlr);
};

#endif // SERVERBUILDER_H
