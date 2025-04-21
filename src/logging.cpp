#include "ILogger.h"
#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>

void logConnection(ILogger& l, struct sockaddr_storage addr) {
    if (addr.ss_family == AF_INET) {
        std::stringstream info;
        sockaddr_in* addr_in = (sockaddr_in*)&addr;
        unsigned char* ip = reinterpret_cast<unsigned char*>(&addr_in->sin_addr.s_addr);
        info << "Connection accepted from IP: " << static_cast<int>(ip[0]) << '.' << static_cast<int>(ip[1]) << '.'
             << static_cast<int>(ip[2]) << '.' << static_cast<int>(ip[3]) << ", Port: " << ntohs(addr_in->sin_port);
        l.log("INFO", info.str());
    }
    if (addr.ss_family == AF_INET6) {
        std::stringstream info;
        char ipstr[INET6_ADDRSTRLEN];
        sockaddr_in6* addr_in6 = (sockaddr_in6*)&addr;
        inet_ntop(AF_INET6, &(addr_in6->sin6_addr), ipstr, INET6_ADDRSTRLEN);
        info << "Connection accepted from IP: " << std::string(ipstr) << ", Port: " << ntohs(addr_in6->sin6_port);
        l.log("INFO", info.str());
    }
}

void logDisconnect(ILogger& logger, sockaddr_storage addr) {
    if (addr.ss_family == AF_INET) {
        std::stringstream info;
        sockaddr_in* addr_in = (sockaddr_in*)&addr;
        unsigned char* ip = reinterpret_cast<unsigned char*>(&addr_in->sin_addr.s_addr);
        info << "Disconnect IP: " << static_cast<int>(ip[0]) << '.' << static_cast<int>(ip[1]) << '.'
             << static_cast<int>(ip[2]) << '.' << static_cast<int>(ip[3]) << ", Port: " << ntohs(addr_in->sin_port);
        logger.log("INFO", info.str());
    }
}
