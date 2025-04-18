#include "EPollManager.h"
#include "ILogger.h"
#include "Logger.h"
#include "utils.h"
#include <cstring>
#include <gtest/gtest.h>
#include <netdb.h>
#include <netinet/in.h>

void getSvrAddrInfo(const char* node, const char* port, struct addrinfo** addrInfo) {
    int status;
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((status = getaddrinfo(node, port, &hints, addrInfo) == -1))
        std::cerr << "getaddrinfo error: " << gai_strerror(status);
}

int newSocket(const char* node, const char* port) {
    struct addrinfo* addrInfo;
    getSvrAddrInfo(node, port, &addrInfo);

    int socketfd = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    if (socketfd < 0) {
        perror("socket");
        exit(1);
    }
    int yes = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    if (bind(socketfd, addrInfo->ai_addr, addrInfo->ai_addrlen) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(addrInfo);
    return socketfd;
}

int newListeningSocket1(const char* node, const char* port) {
    int socketfd = newSocket(node, port);
    if (listen(socketfd, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    return socketfd;
}

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
