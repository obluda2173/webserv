// #include "ILogger.h"
// #include "Logger.h"
// #include <gtest/gtest.h>

#include "ConnectionHandler.h"
#include "EpollIONotifier.h"
#include "test_mocks.h"
#include "utils.h"
TEST(ConnectionHandlerTest, firstTest) {
    ILogger* logger = new MockLogger();
    EpollIONotifier* epollMngr = new EpollIONotifier(*logger);

    int serverfd = newListeningSocket(NULL, "8080");
    epollMngr->add(serverfd, READY_TO_READ);

    ConnectionInfo connInfo;
    connInfo.fd = serverfd;
    connInfo.type = PORT_SOCKET;

    // // TODO: need to add the address and port
    // IConnectionHandler* connHdlr = new ConnectionHandler(logger, epollMngr);

    int clientfd = newSocket("127.0.0.2", "8081");

    (void)clientfd;
    // // connHdlr.handleConnection

    close(clientfd);
    close(serverfd);
    delete epollMngr;
    delete logger;
}
