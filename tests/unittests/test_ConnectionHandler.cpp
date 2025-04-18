#include "EPollManager.h"
#include "ILogger.h"
#include "Logger.h"
#include "utils.h"
#include <gtest/gtest.h>

TEST(ConnectionHandlerTest, firstTest) {
    ILogger* logger = new Logger();
    EPollManager* epollMngr = new EPollManager(logger);

    int serverfd = newListeningSocket1(NULL, "8080");
    ConnectionInfo connInfo;
    connInfo.fd = serverfd;
    connInfo.type = PORT_SOCKET;
    // // TODO: need to add the address and port
    epollMngr->add(serverfd, &connInfo, READY_TO_READ);

    // IConnectionHandler* connHdlr = new ConnectionHandler(logger, epollMngr);

    int clientfd = newSocket("127.0.0.2", "8081");

    (void)clientfd;
    // // connHdlr.handleConnection

    close(clientfd);
    close(serverfd);
    delete epollMngr;
    delete logger;
}
