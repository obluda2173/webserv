#include "ConfigParser.h"
#include "ConnectionHandler.h"
#include "EpollIONotifier.h"
#include "GetHandler.h"
#include "Logger.h"
#include "Router.h"
#include "ServerBuilder.h"
#include "UploadHandler.h"

int main() {

    IConfigParser* cfgPrsr = new ConfigParser("tests/end_to_end_tests/Config1/config1.conf");
    Logger* logger = new Logger();
    EpollIONotifier* ioNotifier = new EpollIONotifier(*logger);

    std::map< std::string, IHandler* > hdlrs;
    hdlrs["GET"] = new GetHandler();
    hdlrs["POST"] = new UploadHandler();
    Router router = newRouter(cfgPrsr->getServersConfig(), hdlrs);

    std::map< int, IRouter* > routers;
    routers[8080] = &router;
    ConnectionHandler* connHdlr = new ConnectionHandler(routers, *logger, *ioNotifier);

    Server* svr = ServerBuilder().setLogger(logger).setIONotifier(ioNotifier).setConnHdlr(connHdlr).build();

    std::vector< std::string > ports;
    ports.push_back("8080");

    svr->start(ports);

    return 0;
}
