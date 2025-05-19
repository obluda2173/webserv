#include "EpollIONotifier.h"
#include "IIONotifier.h"
#include "Logger.h"
#include "test_stubs.h"
#include <cstring>
#include <gtest/gtest.h>
#include <netdb.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <utils.h>

static const int WRITE_END = 1;
static const int READ_END = 0;

TEST(IONotifierTest, timeoutFirstTest) {
    ILogger* logger = new Logger();
    StubClock* clock = new StubClock();
    IIONotifier* ioNotifier = new EpollIONotifier(*logger, clock);
    std::vector< t_notif > notifs;

    int pipefds[2];
    ASSERT_NE(pipe(pipefds), -1) << strerror(errno);

    ioNotifier->add(pipefds[READ_END], 20);
    clock->advance(21);
    notifs = ioNotifier->wait();
    EXPECT_EQ(notifs.size(), 1) << "no notifiactions found";
    EXPECT_EQ(notifs[0].fd, pipefds[READ_END]) << "no notifiactions found";
    EXPECT_EQ(notifs[0].notif, TIMEOUT) << "no notifiactions found";

    close(pipefds[WRITE_END]);
    close(pipefds[READ_END]);
    delete ioNotifier;
    delete logger;
}

TEST(IONotifierTest, timeoutSecondTest) {
    ILogger* logger = new Logger();
    StubClock* clock = new StubClock();
    IIONotifier* ioNotifier = new EpollIONotifier(*logger, clock);
    std::vector< t_notif > notifs;

    int pipefds[2];
    ASSERT_NE(pipe(pipefds), -1) << strerror(errno);

    ioNotifier->add(pipefds[READ_END], 20);
    clock->advance(21);
    notifs = ioNotifier->wait();
    EXPECT_EQ(notifs.size(), 1) << "no notifiactions found";
    EXPECT_EQ(notifs[0].fd, pipefds[READ_END]) << "no notifiactions found";
    EXPECT_EQ(notifs[0].notif, TIMEOUT) << "no notifiactions found";

    close(pipefds[WRITE_END]);
    close(pipefds[READ_END]);
    delete ioNotifier;
    delete logger;
}
