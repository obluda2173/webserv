#ifndef LOGGING_H
#define LOGGING_H

#include "ILogger.h"
#include <netinet/in.h>

void logConnection(ILogger& l, sockaddr_storage addr);
void logDisconnect(ILogger& logger, sockaddr_storage addr);

#endif // LOGGING_H
