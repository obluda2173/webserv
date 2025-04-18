#include "ConnectionHandler.h"
#include "EPollManager.h"
#include "Listener.h"
#include "test_fixtures.h"
#include "test_main.h"
#include "utils.h"
#include "gtest/gtest.h"
#include <netinet/in.h>

class ListenerTest : public ::testing::Test {};

TEST_F(ListenerTest, closingAConnection) {
    int openFdsBegin = countOpenFileDescriptors();
    {
        int svrPort = 8080;
        int sfd1 = newListeningSocket(svrPort);

        int clientPort = 12345;
        std::string clientIp = "127.0.0.2";
        int clientfd = getClientSocket(clientIp, clientPort);

        MockLogger* mLogger = new MockLogger;
        EPollManager* epollMngr = new EPollManager(mLogger);
        ConnectionHandler* connHdlr = new ConnectionHandler(mLogger, epollMngr);
        Listener listener(mLogger, connHdlr, epollMngr);
        listener.add(sfd1);

        std::thread listenerThread;
        listenerThread = std::thread(&Listener::listen, &listener);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        EXPECT_CALL(*mLogger,
                    log("INFO", "Connection accepted from IP: " + clientIp + ", Port: " + std::to_string(clientPort)));

        struct sockaddr_in svrAddr;

        setSvrAddr(svrAddr, svrPort);
        svrAddr.sin_family = AF_INET;
        svrAddr.sin_port = htons(svrPort);
        ASSERT_GT(inet_pton(AF_INET, "127.0.0.1", &(svrAddr.sin_addr)), 0) << "inet_pton: " << strerror(errno);
        ASSERT_EQ(connect(clientfd, (sockaddr*)&svrAddr, sizeof(svrAddr)), 0) << "connect: " << strerror(errno);

        EXPECT_CALL(*mLogger, log("INFO", "Disconnect IP: " + clientIp + ", Port: " + std::to_string(clientPort)));
        close(clientfd);

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        listener.stop();
        listenerThread.join();
        close(sfd1);
        delete epollMngr;
        delete mLogger;
        delete connHdlr;
    }
    EXPECT_EQ(countOpenFileDescriptors(), openFdsBegin);
}

TEST_F(ListenerTest, multiplePortsTestWithLogging) {
    int openFdsBegin = countOpenFileDescriptors();
    {
        int sfd1 = newListeningSocket(8070);
        int sfd2 = newListeningSocket(8071);

        MockLogger* mLogger = new MockLogger;
        EPollManager* epollMngr = new EPollManager(mLogger);
        ConnectionHandler* connHdlr = new ConnectionHandler(mLogger, epollMngr);
        Listener listener(mLogger, connHdlr, epollMngr);
        listener.add(sfd1);
        listener.add(sfd2);

        std::thread listenerThread;
        listenerThread = std::thread(&Listener::listen, &listener);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        testMultipleConnectionsWithLogging(mLogger, 8070, 100);
        testMultipleConnectionsWithLogging(mLogger, 8071, 100);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        listener.stop();
        listenerThread.join();
        close(sfd1);
        close(sfd2);
        delete epollMngr;
        delete mLogger;
        delete connHdlr;
    }
    EXPECT_EQ(countOpenFileDescriptors(), openFdsBegin);
}

TEST_F(ListenerTest, multiplePortsTestWoLogging) {
    int openFdsBegin = countOpenFileDescriptors();
    {
        int sfd1 = newListeningSocket(8070);
        int sfd2 = newListeningSocket(8071);

        ILogger* logger = new StubLogger();
        EPollManager* epollMngr = new EPollManager(logger);
        ConnectionHandler* connHdlr = new ConnectionHandler(logger, epollMngr);
        Listener listener(logger, connHdlr, epollMngr);
        listener.add(sfd1);
        listener.add(sfd2);

        std::thread listenerThread;
        listenerThread = std::thread(&Listener::listen, &listener);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        testMultipleConnections(8070, 100);
        testMultipleConnections(8071, 100);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        listener.stop();
        close(sfd1);
        close(sfd2);
        listenerThread.join();
        delete epollMngr;
        delete logger;
        delete connHdlr;
    }
    EXPECT_EQ(countOpenFileDescriptors(), openFdsBegin);
}
