#ifndef LOGGING_H
#define LOGGING_H

#include "ILogger.h"
#include <netinet/in.h>

void logConnection(ILogger& l, sockaddr_storage addr);
void logDisconnect(ILogger& logger, sockaddr_storage addr);
std::string getIpv4String(struct sockaddr_in* addr_in);
std::string getIpv6String(struct sockaddr_in6& addr);

#endif // LOGGING_H
