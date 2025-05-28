#include "ConfigParser.h"
#include "ConnectionHandler.h"
#include "EpollIONotifier.h"
#include "Logger.h"
#include "Router.h"
#include "ServerBuilder.h"
#include "utils.h"
#include <csignal>
#include <iostream>

// Global flag for signal handling
volatile sig_atomic_t g_running = 1;

// Signal handler function
void signalHandler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        g_running = 0;
    }
}

int main(int argc, char** argv) {
    // Set up signal handling
    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);  // Handle Ctrl+C
    sigaction(SIGTERM, &sa, NULL); // Handle termination signal

    if (argc != 2) {
        std::cerr << "We expect exactly one Configuration File!" << std::endl;
        exit(1);
    }

    std::string filename = std::string(argv[1]);
    IConfigParser* cfgPrsr = new ConfigParser(filename);
    std::vector< ServerConfig > svrCfgs = cfgPrsr->getServersConfig();
    delete cfgPrsr;

    mustTranslateToRealIps(svrCfgs);
    std::map< std::string, IRouter* > routers = buildRouters(svrCfgs);

    Logger* logger = new Logger();
    EpollIONotifier* ioNotifier = new EpollIONotifier(*logger);
    ConnectionHandler* connHdlr = new ConnectionHandler(routers, *logger, *ioNotifier);
    Server* svr = ServerBuilder().setLogger(logger).setIONotifier(ioNotifier).setConnHdlr(connHdlr).build();

    std::set< std::pair< std::string, std::string > > addrAndPorts = fillAddrAndPorts(svrCfgs);

    svr->start(addrAndPorts, &g_running);

    svr->stop();
    return 0;
}
