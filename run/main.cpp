#include "ConfigParser.h"
#include "ConnectionHandler.h"
#include "EpollIONotifier.h"
#include "GetHandler.h"
#include "Logger.h"
#include "Router.h"
#include "ServerBuilder.h"
#include "UploadHandler.h"
#include "utils.h"

int main() {

    IConfigParser* cfgPrsr = new ConfigParser("tests/end_to_end_tests/Config1/config1.conf");
    Logger* logger = new Logger();
    EpollIONotifier* ioNotifier = new EpollIONotifier(*logger);

    std::map< std::string, IRouter* > routers;
    std::set< std::pair<std::string, std::string > > addrPorts;
    std::vector< ServerConfig > svrCfgs = cfgPrsr->getServersConfig();
    for (size_t i = 0; i < svrCfgs.size(); i++) {
        ServerConfig svrCfg = svrCfgs[i];
        for (std::map< std::string, int >::iterator it = svrCfg.listen.begin(); it != svrCfg.listen.end(); it++) {
            if (routers.find(it->first + ":" + to_string(it->second)) != routers.end()) {
                addSvrToRouter(routers[it->first + ":" + to_string(it->second)], svrCfg);
            } else {
                std::map< std::string, IHandler* > hdlrs;
                hdlrs["GET"] = new GetHandler();
                hdlrs["POST"] = new UploadHandler();
                IRouter* r = new Router(hdlrs);
                addSvrToRouter(r, svrCfg);
                routers[it->first + ":" + to_string(it->second)] = r;

                addrPorts.insert(std::pair<std::string, std::string>(it->first, to_string(it->second)));
            }
        }
    }

    ConnectionHandler* connHdlr = new ConnectionHandler(routers, *logger, *ioNotifier);

    Server* svr = ServerBuilder().setLogger(logger).setIONotifier(ioNotifier).setConnHdlr(connHdlr).build();


    svr->start(addrPorts);

    return 0;
}
