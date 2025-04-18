#include "ConnectionHandler.h"
#include "EPollManager.h"
#include "IConnectionHandler.h"
#include "ILogger.h"
#include "Logger.h"
#include "test_main.h"
#include "utils.h"
#include <cstring>
#include <gtest/gtest.h>
#include <netdb.h>
#include <netinet/in.h>

TEST(ConnectionHandlerTest, firstTest) {
    ILogger* logger = new Logger();
    EPollManager* epollMngr = new EPollManager(logger);

    int status;
    struct addrinfo hints;
    struct addrinfo* svrInfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((status = getaddrinfo(NULL, "8080", &hints, &svrInfo) == -1))
        std::cerr << "getaddrinfo error: " << gai_strerror(status);

    char ipstr[INET6_ADDRSTRLEN];
    struct sockaddr_in* ipv4 = (struct sockaddr_in*)svrInfo->ai_addr;
    void* addr = &(ipv4->sin_addr);
    // void* ipv4 = &(((struct sockaddr_in*)svrInfo->ai_addr)->sin_addr);
    inet_ntop(svrInfo->ai_family, addr, ipstr, sizeof ipstr);

    freeaddrinfo(svrInfo);
    std::cout << ipstr << std::endl;
    // int socket = newListeningSocket(8080);
    // ConnectionInfo connInfo;
    // connInfo.fd = socket;
    // connInfo.type = PORT_SOCKET;
    // // TODO: need to add the address and port
    // epollMngr->add(socket, &connInfo, READY_TO_READ);

    // IConnectionHandler* connHdlr = new ConnectionHandler(logger, epollMngr);

    // int clientfd = getClientSocket("127.0.0.2", 8081);

    // // connHdlr.handleConnection

    delete epollMngr;
    delete logger;
}
