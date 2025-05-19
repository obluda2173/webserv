#include "EpollIONotifier.h"
#include "IIONotifier.h"
#include "Logger.h"
#include "test_main.h"
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
    int openFdsBegin = countOpenFileDescriptors();
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
    EXPECT_EQ(openFdsBegin, countOpenFileDescriptors());
}

// TEST(IONotifierTest, timeoutSecondTest) {
//     int openFdsBegin = countOpenFileDescriptors();
//     ILogger* logger = new Logger();
//     StubClock* clock = new StubClock();
//     IIONotifier* ioNotifier = new EpollIONotifier(*logger, clock);
//     std::vector< t_notif > notifs;

//     int time = 0;
//     int nbrPipes = 10;
//     std::vector< int[2] > pipes(nbrPipes);
//     for (int i = 0; i < nbrPipes; i++) {
//         ASSERT_NE(pipe(pipes[i]), -1);
//         ioNotifier->add(pipes[i][READ_END], 20);
//         clock->advance(++time);
//     }

//     while (time < 20)
//         clock->advance(++time);

//     notifs = ioNotifier->wait();
//     EXPECT_EQ(notifs.size(), 1) << "no notifiactions found";
//     EXPECT_EQ(notifs[0].fd, pipes[0][READ_END]) << "not the correct filedescriptor";
//     // EXPECT_EQ(notifs[0].notif, TIMEOUT) << "no notifiactions found";

//     for (int i = 0; i < nbrPipes; i++) {
//         close(pipes[i][READ_END]);
//         close(pipes[i][WRITE_END]);
//     }
//     delete ioNotifier;
//     delete logger;
//     EXPECT_EQ(openFdsBegin, countOpenFileDescriptors());
// }
