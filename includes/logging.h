#ifndef LOGGING_H
#define LOGGING_H

#include "ILogger.h"

void logConnection(ILogger *l, struct sockaddr_in addr);
void logDisconnect(ILogger* logger, sockaddr_in addr);

#endif // LOGGING_H
