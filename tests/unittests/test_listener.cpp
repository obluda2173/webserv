#include "Listener.h"
#include "test_fixtures.h"
#include "test_main.h"
#include "utils.h"

class ListenerTest : public ::testing::Test {};

TEST_F(ListenerTest, multiplePortsTest) {
    int sfd1 = new_socket(8070);
    int sfd2 = new_socket(8071);

    MockLogger* mLogger = new MockLogger;
    Listener listener(mLogger);
    listener.add(sfd1);
    listener.add(sfd2);

    std::thread listenerThread;
    listenerThread = std::thread(&Listener::listen, &listener);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    testMultipleConnectionsWithLogging(mLogger, 8070);
    testMultipleConnectionsWithLogging(mLogger, 8071);
    listener.stop();
    listenerThread.join();
    delete mLogger;
}
