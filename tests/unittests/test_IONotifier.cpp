#include "EpollIONotifier.h"
#include "IIONotifier.h"
#include "test_stubs.h"
#include <cstring>
#include <gtest/gtest.h>
#include <netdb.h>
#include <sys/socket.h>
#include <utils.h>

TEST(TestIONotifier, testBrokenConnection) {
    ILogger* logger = new StubLogger();
    IIONotifier* ioNotifier = new EpollIONotifier(*logger);

    struct addrinfo* svrAddrInfo;
    getAddrInfoHelper(NULL, "8080", AF_INET, &svrAddrInfo);
    int fd = newListeningSocket(svrAddrInfo, 5);
    freeaddrinfo(svrAddrInfo);

    ioNotifier->add(fd, READY_TO_READ);

    shutdown(fd, SHUT_RDWR);

    int fds;
    e_notif notifs;
    ioNotifier->wait(&fds, &notifs);

    ASSERT_EQ(BROKEN_CONNECTION, notifs);

    delete ioNotifier;
    delete logger;
}

TEST(TestIONotifier, testBeingAbleToReadButBrokenConnection) {
    ILogger* logger = new StubLogger();
    IIONotifier* ioNotifier = new EpollIONotifier(*logger);

    struct addrinfo* svrAddrInfo;
    getAddrInfoHelper(NULL, "8080", AF_INET, &svrAddrInfo);
    int fd = newListeningSocket(svrAddrInfo, 5);
    freeaddrinfo(svrAddrInfo);

    std::string msg = "A message written into fd";
    send;
    ioNotifier->add(fd, READY_TO_READ);

    shutdown(fd, SHUT_RDWR);

    int fds;
    e_notif notifs;
    ioNotifier->wait(&fds, &notifs);

    ASSERT_EQ(BROKEN_CONNECTION, notifs);

    delete ioNotifier;
    delete logger;
}
