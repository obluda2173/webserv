#include "ILogger.h"
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>

void logConnection(ILogger& l, struct sockaddr_in addr) {
    std::stringstream info;
    unsigned char* ip = reinterpret_cast<unsigned char*>(&addr.sin_addr.s_addr);
    info << "Connection accepted from IP: " << static_cast<int>(ip[0]) << '.' << static_cast<int>(ip[1]) << '.'
         << static_cast<int>(ip[2]) << '.' << static_cast<int>(ip[3]) << ", Port: " << ntohs(addr.sin_port);
    l.log("INFO", info.str());
}

void logDisconnect(ILogger& logger, sockaddr_in addr) {
    std::stringstream info;
    unsigned char* ip = reinterpret_cast<unsigned char*>(&addr.sin_addr.s_addr);
    info << "Disconnect IP: " << static_cast<int>(ip[0]) << '.' << static_cast<int>(ip[1]) << '.'
         << static_cast<int>(ip[2]) << '.' << static_cast<int>(ip[3]) << ", Port: " << ntohs(addr.sin_port);
    logger.log("INFO", info.str());
}

// void logDisconnectStorage(ILogger& logger, sockaddr_in addr) {
//     std::stringstream info;
//     unsigned char* ip = reinterpret_cast<unsigned char*>(&addr.sin_addr.s_addr);
//     info << "Disconnect IP: " << static_cast<int>(ip[0]) << '.' << static_cast<int>(ip[1]) << '.'
//          << static_cast<int>(ip[2]) << '.' << static_cast<int>(ip[3]) << ", Port: " << ntohs(addr.sin_port);
//     logger.log("INFO", info.str());
// }
