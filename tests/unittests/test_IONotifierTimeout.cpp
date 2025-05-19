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
};

TEST_F(IONotifierTestTimeout, addAndDeleteFds) {
    std::vector< t_notif > notifs;

    int time = 0;
    for (int i = 0; i < _nbrOfPipes; i++) {
        ASSERT_NE(pipe(_pipes[i]), -1);
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
        EXPECT_EQ(notifs[0].fd, _pipes[i][READ_END]) << "not the correct filedescriptor";
        EXPECT_EQ(notifs[0].notif, TIMEOUT) << "no notifiactions found";
        _ioNotifier->del(notifs[0].fd);

        time++;
        _clock->advance(1);
    }
}
