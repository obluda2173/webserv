#include "EpollIONotifier.h"
#include "IIONotifier.h"
#include "test_main.h"
#include "test_stubs.h"
#include <gtest/gtest.h>
#include <netdb.h>
#include <sys/socket.h>
#include <utils.h>

TEST(testIONotifier, testBrokenConnection) {
    ILogger* logger = new StubLogger();
    IIONotifier* ioNotifier = new EpollIONotifier(*logger);

    struct addrinfo* svrAddrInfo;
    getAddrInfoHelper(NULL, "8080", AF_INET, &svrAddrInfo);
    int fd = newListeningSocket(svrAddrInfo, 5);

    ioNotifier->add(fd, READY_TO_READ);

    shutdown(fd, SHUT_RDWR);

    int fds;
    e_notif notifs;
    ioNotifier->wait(&fds, &notifs);

    ASSERT_EQ(BROKEN_CONNECTION, notifs);
}
