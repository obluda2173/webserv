#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include "EPollManager.h"
#include "IConnectionHandler.h"

class ConnectionHandler : public IConnectionHandler {
  private:
    ILogger* _logger;
    EPollManager* _epollMngr;
		void _addClientConnection(int conn, struct sockaddr* theirAddr);
		void _removeClientConnection(ConnectionInfo* connInfo);

  public:
    ConnectionHandler(ILogger*, EPollManager*);
    ~ConnectionHandler(void);
    void handleConnection(ConnectionInfo* connInfo);
};

#endif // CONNECTIONHANDLER_H
