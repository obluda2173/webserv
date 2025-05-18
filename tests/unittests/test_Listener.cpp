#include "IConnectionHandler.h"
#include "IIONotifier.h"
#include "test_ListenerFixtures.h"
#include "test_main.h"
#include "test_stubs.h"
#include "utils.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

class StubConnectionHandler : public IConnectionHandler {
  public:
    std::set< std::pair< int, e_notif > > _calledWith;
    int handleConnection(int conn, e_notif notif) {
        _calledWith.insert(std::pair< int, e_notif >(conn, notif));
        return 0;
    }
};

TEST(ListenerTestSimple, connHdlrGetsCalled) {
    ILogger* logger = new StubLogger();
    EpollIONotifier* ioNotifier = new EpollIONotifier(*logger);
    struct addrinfo* svrAddrInfo;
    getAddrInfoHelper(NULL, std::to_string(8080).c_str(), AF_INET, &svrAddrInfo);
    int portfd = newListeningSocket(svrAddrInfo, 5);
    ioNotifier->add(portfd);

    StubConnectionHandler* connHdlr = new StubConnectionHandler();
    Listener* listener = new Listener(*logger, connHdlr, ioNotifier);
    listener->add(portfd);
    std::thread listenerThread = std::thread(&Listener::listen, listener);

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    std::string clientPort = "12345";
    std::string clientIp = "127.0.0.3";
    int clientfd = newSocket(clientIp, clientPort, AF_INET);
    ASSERT_EQ(connect(clientfd, svrAddrInfo->ai_addr, svrAddrInfo->ai_addrlen), 0) << "connect: " << strerror(errno);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    ASSERT_EQ(connHdlr->_calledWith.size(), 1);
    ASSERT_TRUE(connHdlr->_calledWith.find(std::pair< int, e_notif >{portfd, READY_TO_READ}) !=
                connHdlr->_calledWith.end());

    struct sockaddr_storage theirAddr;
    int addrlen = sizeof(theirAddr);
    int connfd = accept(portfd, (struct sockaddr*)&theirAddr, (socklen_t*)&addrlen);

    ioNotifier->add(connfd);
    ASSERT_FALSE(connHdlr->_calledWith.find(std::pair< int, e_notif >{connfd, READY_TO_READ}) !=
                 connHdlr->_calledWith.end());
    send(clientfd, "message", 7, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    ASSERT_TRUE(connHdlr->_calledWith.find(std::pair< int, e_notif >{connfd, READY_TO_READ}) !=
                connHdlr->_calledWith.end());
    char buffer[1024];
    recv(connfd, buffer, 1024, 0);

    ASSERT_FALSE(connHdlr->_calledWith.find(std::pair< int, e_notif >{connfd, READY_TO_WRITE}) !=
                 connHdlr->_calledWith.end());
    ioNotifier->modify(connfd, READY_TO_WRITE);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    ASSERT_TRUE(connHdlr->_calledWith.find(std::pair< int, e_notif >{connfd, READY_TO_WRITE}) !=
                connHdlr->_calledWith.end());

    connHdlr->_calledWith = {};
    ASSERT_FALSE(connHdlr->_calledWith.find(std::pair< int, e_notif >{connfd, READY_TO_READ}) !=
                 connHdlr->_calledWith.end());
    ioNotifier->modify(connfd, READY_TO_READ);
    send(clientfd, "message", 7, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    ASSERT_TRUE(connHdlr->_calledWith.find(std::pair< int, e_notif >{connfd, READY_TO_READ}) !=
                connHdlr->_calledWith.end());
    recv(connfd, buffer, 1024, 0);

    connHdlr->_calledWith = {};
    struct linger so_linger;
    so_linger.l_onoff = 1;  // Enable linger
    so_linger.l_linger = 0; // Zero timeout - immediate RST

    setsockopt(clientfd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
    close(clientfd); // This should now trigger EPOLLHUP on the server side
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    ASSERT_TRUE(connHdlr->_calledWith.find(std::pair< int, e_notif >{connfd, BROKEN_CONNECTION}) !=
                connHdlr->_calledWith.end());

    // connHdlr->_calledWith = {};
    // close(clientfd);
    // // shutdown(clientfd, SHUT_WR);
    // std::this_thread::sleep_for(std::chrono::milliseconds(20));
    // ASSERT_EQ(connHdlr->_calledWith.size(), 1);
    // ASSERT_TRUE(connHdlr->_calledWith.find(std::pair< int, e_notif >{connfd, CLIENT_HUNG_UP}) !=
    //             connHdlr->_calledWith.end());
    // size_t r = recv(connfd, buffer, 1024, 0);
    // ASSERT_EQ(0, r);

    freeaddrinfo(svrAddrInfo);

    listener->stop();
    if (listenerThread.joinable())
        listenerThread.join();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    delete listener;
    delete logger;
}

TEST_P(ListenerTestWithMockLogging, closingAConnection) {
    using ::testing::_;
    EXPECT_CALL(*_logger, log(_, _)).Times(testing::AnyNumber());
    std::vector< int > ports = GetParam();
    for (size_t i = 0; i < ports.size(); i++) {
        int port = ports[i];
        std::string clientPort = "12345";
        std::string clientIp = "127.0.0.3";
        int clientfd = newSocket(clientIp, clientPort, AF_INET);

        struct addrinfo* svrAddrInfo;
        getAddrInfoHelper(NULL, std::to_string(port).c_str(), AF_INET, &svrAddrInfo);
        // Either don't expect this at all, or make it more flexible
        testing::Expectation acceptLog =
            EXPECT_CALL(*_logger, log("INFO", "Connection accepted from IP: " + clientIp + ", Port: " + clientPort))
                .Times(1);
        ASSERT_EQ(connect(clientfd, svrAddrInfo->ai_addr, svrAddrInfo->ai_addrlen), 0)
            << "connect: " << strerror(errno);
        freeaddrinfo(svrAddrInfo);

        EXPECT_CALL(*_logger, log("INFO", "Disconnect IP: " + clientIp + ", Port: " + clientPort)).Times(1);
        close(clientfd);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

TEST_P(ListenerTestWithMockLogging, multipleConnections) {
    using ::testing::_;
    EXPECT_CALL(*_logger, log(_, _)).Times(testing::AnyNumber());
    std::vector< int > ports = GetParam();
    for (size_t i = 0; i < ports.size(); i++) {
        int port = ports[i];
        testMultipleConnectionsWithLogging(_logger, std::to_string(port), 50);
    }
}

INSTANTIATE_TEST_SUITE_P(multiplePorts, ListenerTestWithMockLogging,
                         ::testing::Values(std::vector< int >{8080}, std::vector< int >{8080, 8081}));

TEST_P(ListenerTestWoMockLogging, multiplePortsTestWoLogging) {
    std::vector< int > ports = GetParam();
    for (size_t i = 0; i < ports.size(); i++) {
        int port = ports[i];
        testMultipleConnections(std::to_string(port), 50);
    }
}

INSTANTIATE_TEST_SUITE_P(multiplePorts, ListenerTestWoMockLogging,
                         ::testing::Values(std::vector< int >{8080}, std::vector< int >{8080, 8081}));
