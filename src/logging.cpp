#include "ILogger.h"
#include <arpa/inet.h>
#include <sstream>

void logConnection(ILogger *l, struct sockaddr_in addr) {
    std::stringstream info;
    info << "Connection accepted from IP: " << inet_ntoa(addr.sin_addr) << ", Port: " << ntohs(addr.sin_port);
    l->log("INFO", info.str());
}
