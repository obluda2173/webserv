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

class IONotifierTestTimeout : public testing::Test {
  protected:
    int _openFdsBegin;
    ILogger* _logger;
    StubClock* _clock;
    IIONotifier* _ioNotifier;
    int _nbrOfPipes;
    std::vector< int[2] > _pipes;

  public:
    virtual void SetUp() override {
        _openFdsBegin = countOpenFileDescriptors();
        _logger = new Logger();
        _clock = new StubClock();
        _ioNotifier = new EpollIONotifier(*_logger, _clock);
        _nbrOfPipes = 10;
        _pipes = std::vector< int[2] >(_nbrOfPipes);

        for (int i = 0; i < _nbrOfPipes; i++)
            ASSERT_NE(pipe(_pipes[i]), -1);
    }

    virtual void TearDown() override {
        for (int i = 0; i < _nbrOfPipes; i++) {
            close(_pipes[i][READ_END]);
            close(_pipes[i][WRITE_END]);
        }
        delete _ioNotifier;
        delete _logger;
        EXPECT_EQ(_openFdsBegin, countOpenFileDescriptors());
    }

    void _readOutFd(int fd) {
        std::vector< t_notif > notifs;
        char buffer[1024];
        read(fd, buffer, 1024);
        notifs = _ioNotifier->wait();
        EXPECT_EQ(notifs.size(), 0) << "notification found";
    }

    void _writeHelloToPipe(int fd) { write(fd, "hello", 5); }
};

TEST_F(IONotifierTestTimeout, addAndDeleteFds) {
    std::vector< t_notif > notifs;

    int time = 0;
    for (int i = 0; i < _nbrOfPipes; i++) {
        _ioNotifier->add(_pipes[i][READ_END], 20);
        _clock->advance(1);
        time++;
    }

    while (time++ < 20)
        _clock->advance(1);
    // now at time 20

    for (size_t i = 0; i < _pipes.size(); i++) {
        notifs = _ioNotifier->wait();
        EXPECT_EQ(notifs.size(), 1) << "no notifiactions found";
        expectNotif(notifs[0], _pipes[i][READ_END], TIMEOUT);
        _ioNotifier->del(notifs[0].fd);

        time++;
        _clock->advance(1);
    }
}

TEST_F(IONotifierTestTimeout, resetActivity) {
    std::vector< t_notif > notifs;

    _ioNotifier->add(_pipes[0][READ_END], 20);
    _clock->advance(10);

    _writeHelloToPipe(_pipes[0][WRITE_END]);
    notifs = _ioNotifier->wait();
    EXPECT_EQ(notifs.size(), 1) << "no notifiactions found";
    expectNotif(notifs[0], _pipes[0][READ_END], READY_TO_READ);

    _readOutFd(_pipes[0][READ_END]);

    // expect no more notifs
    notifs = _ioNotifier->wait();
    EXPECT_EQ(notifs.size(), 0) << "there was a notification";

    // now there it's the original timeout but we're going to expect no timeout
    _clock->advance(10);
    notifs = _ioNotifier->wait();
    EXPECT_EQ(notifs.size(), 0) << "there was a notification";

    // while (time++ < 20)
    //     _clock->advance(1);
    // // now at time 20

    // for (size_t i = 0; i < _pipes.size(); i++) {
    //     notifs = _ioNotifier->wait();
    //     EXPECT_EQ(notifs.size(), 1) << "no notifiactions found";
    //     EXPECT_EQ(notifs[0].fd, _pipes[i][READ_END]) << "not the correct filedescriptor";
    //     EXPECT_EQ(notifs[0].notif, TIMEOUT) << "no notifiactions found";
    //     _ioNotifier->del(notifs[0].fd);

    //     time++;
    //     _clock->advance(1);
    // }
}
