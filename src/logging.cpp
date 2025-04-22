#include "ILogger.h"
#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>

std::string getIpv4String(struct sockaddr_in* addr_in) {
    std::stringstream res;
    unsigned char* ip = reinterpret_cast<unsigned char*>(&addr_in->sin_addr.s_addr);
    res << static_cast<int>(ip[0]) << '.' << static_cast<int>(ip[1]) << '.' << static_cast<int>(ip[2]) << '.'
        << static_cast<int>(ip[3]);
    return res.str();
}

std::string getIpv6String(struct sockaddr_in6& addr) {
    std::stringstream res;
    const uint8_t* bytes = addr.sin6_addr.s6_addr;
    for (int i = 0; i < 16; i += 2) {
        res << std::hex << static_cast<int>(bytes[i]) << static_cast<int>(bytes[i + 1]);
        if (i < 14)
            res << ":";
    }
    return res.str();
}

void logConnection(ILogger& l, struct sockaddr_storage addr) {
    std::stringstream info;
    info << "Connection accepted from IP: ";
    if (addr.ss_family == AF_INET) {
        sockaddr_in* addr_in = (sockaddr_in*)&addr;
        info << getIpv4String(addr_in);
        info << ", Port: " << ntohs(addr_in->sin_port);
    }
    if (addr.ss_family == AF_INET6) {
        sockaddr_in6* addr_in6 = (sockaddr_in6*)&addr;
        info << getIpv6String(*addr_in6);
        info << ", Port: " << ntohs(addr_in6->sin6_port);
    }
    l.log("INFO", info.str());
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
