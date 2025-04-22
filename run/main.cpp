#include "ConnectionHandler.h"
#include "EpollIONotifier.h"
#include "Logger.h"
#include "ServerBuilder.h"

int main() {
    Logger* logger = new Logger();
    EpollIONotifier* ioNotifier = new EpollIONotifier(*logger);
    ConnectionHandler* connHdlr = new ConnectionHandler(*logger, *ioNotifier);

    Server* svr = ServerBuilder().setLogger(logger).setIONotifier(ioNotifier).setConnHdlr(connHdlr).build();

    std::vector<std::string> ports;
    ports.push_back("8080");
    ports.push_back("8081");

    svr->start(ports);

    return 0;
}
