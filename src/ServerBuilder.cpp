#include "ServerBuilder.h"

Server* ServerBuilder::build(void) { return new Server(_logger, _connHdlr, _ioNotifer); }

ServerBuilder& ServerBuilder::setLogger(ILogger* logger) {
    _logger = logger;
    return *this;
}

ServerBuilder& ServerBuilder::setIONotifier(IIONotifier* ioNotifier) {
    _ioNotifer = ioNotifier;
    return *this;
}

ServerBuilder& ServerBuilder::setConnHdlr(IConnectionHandler* connHdlr) {
    _connHdlr = connHdlr;
    return *this;
}
