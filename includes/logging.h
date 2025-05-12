#ifndef LOGGING_H
#define LOGGING_H

#include "ILogger.h"
#include <netinet/in.h>
#include "HttpRequest.h"

void logConnection(ILogger& l, sockaddr_storage addr);
void logDisconnect(ILogger& logger, sockaddr_storage addr);
void logHttpRequest(ILogger& logger, HttpRequest req);

#endif // LOGGING_H
