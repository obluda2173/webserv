#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include "EPollManager.h"
#include "IConnectionHandler.h"

void addClientConnection(ILogger* _logger, EPollManager* _epollMngr, int conn, struct sockaddr_in theirAddr);
void removeClientConnection(ILogger* _logger, EPollManager* _epollMngr, ConnectionInfo* connInfo);

class ConnectionHandler : public IConnectionHandler {
  private:
    ILogger* _logger;
    EPollManager* _epollMngr;

  public:
    ConnectionHandler(ILogger*, EPollManager*);
    ~ConnectionHandler(void);
    void handleConnection(ConnectionInfo* connInfo);
};

#endif // CONNECTIONHANDLER_H
