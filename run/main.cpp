#include "ConfigParser.h"
#include "ConnectionHandler.h"
#include "EpollIONotifier.h"
#include "GetHandler.h"
#include "Logger.h"
#include "Router.h"
#include "ServerBuilder.h"
#include "UploadHandler.h"
#include "utils.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "We expect exactly one Configuration File!" << std::endl;
        exit(1);
    }

    std::string filename = std::string(argv[1]);
    IConfigParser* cfgPrsr = new ConfigParser(filename);
    std::vector< ServerConfig > svrCfgs = cfgPrsr->getServersConfig();
    delete cfgPrsr;

    mustTranslateToRealIps(svrCfgs);

    std::map< std::string, IRouter* > routers;
    std::set< std::pair< std::string, std::string > > addrPorts;
    for (size_t i = 0; i < svrCfgs.size(); i++) {
        ServerConfig svrCfg = svrCfgs[i];
        for (std::map< std::string, int >::iterator it = svrCfg.listen.begin(); it != svrCfg.listen.end(); it++) {
            std::string ip = it->first;
            std::string port = to_string(it->second);
            if (routers.find(ip + ":" + port) != routers.end()) {
                addSvrToRouter(routers[ip + ":" + port], svrCfg);
            } else {
                std::map< std::string, IHandler* > hdlrs;
                hdlrs["GET"] = new GetHandler();
                hdlrs["POST"] = new UploadHandler();
                IRouter* r = new Router(hdlrs);
                addSvrToRouter(r, svrCfg);
                routers[ip + ":" + port] = r;
                addrPorts.insert(std::pair< std::string, std::string >(ip, port));
            }
        }
    }

    Logger* logger = new Logger();
    EpollIONotifier* ioNotifier = new EpollIONotifier(*logger);
    ConnectionHandler* connHdlr = new ConnectionHandler(routers, *logger, *ioNotifier);
    Server* svr = ServerBuilder().setLogger(logger).setIONotifier(ioNotifier).setConnHdlr(connHdlr).build();

    svr->start(addrPorts);

    return 0;
}
