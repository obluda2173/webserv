#include "ConfigParser.h"
#include "ConnectionHandler.h"
#include "EpollIONotifier.h"
#include "GetHandler.h"
#include "Logger.h"
#include "Router.h"
#include "ServerBuilder.h"
#include "UploadHandler.h"
#include "logging.h"
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

    std::map< std::string, IRouter* > routers;
    std::set< std::pair< std::string, std::string > > addrPorts;
    for (size_t i = 0; i < svrCfgs.size(); i++) {
        ServerConfig svrCfg = svrCfgs[i];
        for (std::map< std::string, int >::iterator it = svrCfg.listen.begin(); it != svrCfg.listen.end(); it++) {
            struct addrinfo* svrAddrInfo;
            try {
                getAddrInfoHelper(it->first.c_str(), to_string(it->second).c_str(), AF_INET, &svrAddrInfo);
            } catch (std::runtime_error& e) {
                try {
                    getAddrInfoHelper(it->first.c_str(), to_string(it->second).c_str(), AF_INET6, &svrAddrInfo);
                } catch (std::runtime_error& e2) {
                    std::cerr << "Caught exception: " << e2.what() << std::endl;
                    exit(1);
                }
            }
            std::string addr;
            if (svrAddrInfo->ai_family == AF_INET) {
                addr = getIpv4String((sockaddr_in*)svrAddrInfo->ai_addr);
            } else {
                addr = getIpv6String((sockaddr_in6*)svrAddrInfo->ai_addr);
            }
            freeaddrinfo(svrAddrInfo);

            if (routers.find(addr + ":" + to_string(it->second)) != routers.end()) {
                addSvrToRouter(routers[addr + ":" + to_string(it->second)], svrCfg);
            } else {
                std::map< std::string, IHandler* > hdlrs;
                hdlrs["GET"] = new GetHandler();
                hdlrs["POST"] = new UploadHandler();
                IRouter* r = new Router(hdlrs);
                addSvrToRouter(r, svrCfg);
                routers[addr + ":" + to_string(it->second)] = r;
                addrPorts.insert(std::pair< std::string, std::string >(addr, to_string(it->second)));
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
