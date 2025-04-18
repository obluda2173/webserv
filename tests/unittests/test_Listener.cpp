#include "ConnectionHandler.h"
#include "EPollManager.h"
#include "Listener.h"
#include "test_fixtures.h"
#include "test_main.h"
#include "utils.h"
#include "gtest/gtest.h"
#include <netdb.h>
#include <netinet/in.h>

class ListenerTest : public ::testing::Test {};

TEST_F(ListenerTest, closingAConnection) {
    int openFdsBegin = countOpenFileDescriptors();
    {
        std::string svrPort = "8080";
        int sfd1 = newListeningSocket1(NULL, svrPort.c_str());

        std::string clientPort = "12345";
        std::string clientIp = "127.0.0.3";
        int clientfd = newSocket(clientIp.c_str(), clientPort.c_str());

        MockLogger* mLogger = new MockLogger;
        EPollManager* epollMngr = new EPollManager(mLogger);
        ConnectionHandler* connHdlr = new ConnectionHandler(mLogger, epollMngr);
        Listener listener(mLogger, connHdlr, epollMngr);
        listener.add(sfd1);

        std::thread listenerThread;
        listenerThread = std::thread(&Listener::listen, &listener);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        EXPECT_CALL(*mLogger, log("INFO", "Connection accepted from IP: " + clientIp + ", Port: " + clientPort));

        struct addrinfo* svrAddrInfo;
        getSvrAddrInfo(NULL, svrPort.c_str(), &svrAddrInfo);
        ASSERT_EQ(connect(clientfd, svrAddrInfo->ai_addr, svrAddrInfo->ai_addrlen), 0)
            << "connect: " << strerror(errno);
        freeaddrinfo(svrAddrInfo);

        EXPECT_CALL(*mLogger, log("INFO", "Disconnect IP: " + clientIp + ", Port: " + clientPort));
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
        int sfd1 = newListeningSocket1(NULL, "8070");
        int sfd2 = newListeningSocket1(NULL, "8071");

        MockLogger* mLogger = new MockLogger;
        EPollManager* epollMngr = new EPollManager(mLogger);
        ConnectionHandler* connHdlr = new ConnectionHandler(mLogger, epollMngr);
        Listener listener(mLogger, connHdlr, epollMngr);
        listener.add(sfd1);
        listener.add(sfd2);

        std::thread listenerThread;
        listenerThread = std::thread(&Listener::listen, &listener);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        testMultipleConnectionsWithLogging(mLogger, "8070", 100);
        testMultipleConnectionsWithLogging(mLogger, "8071", 100);
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
        std::string clientPort1 = "8070";
        std::string clientPort2 = "8071";
        int sfd1 = newListeningSocket1(NULL, clientPort1.c_str());
        int sfd2 = newListeningSocket1(NULL, clientPort2.c_str());

        ILogger* logger = new StubLogger();
        EPollManager* epollMngr = new EPollManager(logger);
        ConnectionHandler* connHdlr = new ConnectionHandler(logger, epollMngr);
        Listener listener(logger, connHdlr, epollMngr);
        listener.add(sfd1);
        listener.add(sfd2);

        std::thread listenerThread;
        listenerThread = std::thread(&Listener::listen, &listener);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        testMultipleConnections(clientPort1, 100);
        testMultipleConnections(clientPort2, 100);
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
