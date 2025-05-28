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

class ListenerTest : public ::testing::Test {
  protected:
    void SetUp() override {
        logger = new StubLogger();
        ioNotifier = new EpollIONotifier(*logger);
        connHdlr = new StubConnectionHandler();

        // Set up server socket
        getAddrInfoHelper(NULL, std::to_string(8080).c_str(), AF_INET, &svrAddrInfo);
        portfd = newListeningSocket(svrAddrInfo, 5);
        ioNotifier->add(portfd);

        // Create and start listener
        listener = new Listener(*logger, connHdlr, ioNotifier);
        listener->add(portfd);
        listenerThread = std::thread(&Listener::listen, listener, nullptr);

        // Allow time for listener to start
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    void TearDown() override {
        listener->stop();
        if (listenerThread.joinable())
            listenerThread.join();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        freeaddrinfo(svrAddrInfo);
        delete listener;
        delete logger;
    }

    // Helper functions
    int connectClient(const std::string& ip, const std::string& port) {
        int clientfd = newSocket(ip, port, AF_INET);
        EXPECT_EQ(connect(clientfd, svrAddrInfo->ai_addr, svrAddrInfo->ai_addrlen), 0)
            << "connect: " << strerror(errno);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        return clientfd;
    }

    int acceptConnection() {
        struct sockaddr_storage theirAddr;
        int addrlen = sizeof(theirAddr);
        int connfd = accept(portfd, (struct sockaddr*)&theirAddr, (socklen_t*)&addrlen);
        ioNotifier->add(connfd);
        return connfd;
    }

    void clearHandlerCalls() { connHdlr->_calledWith = {}; }

    bool isHandlerCalledWith(int fd, e_notif notification) {
        return connHdlr->_calledWith.find(std::pair< int, e_notif >{fd, notification}) != connHdlr->_calledWith.end();
    }

    ILogger* logger;
    EpollIONotifier* ioNotifier;
    StubConnectionHandler* connHdlr;
    struct addrinfo* svrAddrInfo;
    int portfd;
    Listener* listener;
    std::thread listenerThread;
};

struct NotificationTestParams {
    e_notif notification;
    std::function< void(int, EpollIONotifier*, int) > setupAction;
    std::string description;
};

class ListenerNotificationTest : public ListenerTest, public ::testing::WithParamInterface< NotificationTestParams > {};

TEST_P(ListenerNotificationTest, HandlesDifferentIONotifications) {
    const auto& params = GetParam();

    // Set up client connection
    int clientfd = connectClient("127.0.0.3", "12345");
    ASSERT_TRUE(isHandlerCalledWith(portfd, READY_TO_READ));

    // Accept connection
    int connfd = acceptConnection();
    clearHandlerCalls();

    // Perform setup action to trigger notification
    params.setupAction(clientfd, ioNotifier, connfd);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    // Verify correct notification was processed
    ASSERT_TRUE(isHandlerCalledWith(connfd, params.notification));

    // Clean up client connection if needed
    if (clientfd > 0 && params.notification != BROKEN_CONNECTION) {
        close(clientfd);
    }
}

// Define test parameters
INSTANTIATE_TEST_SUITE_P(
    ListenerIOEvents, ListenerNotificationTest,
    ::testing::Values(
        NotificationTestParams{READY_TO_READ,
                               [](int clientfd, EpollIONotifier*, int) { send(clientfd, "message", 7, 0); },
                               "Connection is ready to read"},
        NotificationTestParams{
            READY_TO_WRITE,
            [](int, EpollIONotifier* notifier, int connfd) { notifier->modify(connfd, READY_TO_WRITE); },
            "Connection is ready to write"},
        NotificationTestParams{CLIENT_HUNG_UP, [](int clientfd, EpollIONotifier*, int) { shutdown(clientfd, SHUT_WR); },
                               "Client hung up"},
        NotificationTestParams{BROKEN_CONNECTION,
                               [](int clientfd, EpollIONotifier*, int) {
                                   struct linger so_linger;
                                   so_linger.l_onoff = 1;  // Enable linger
                                   so_linger.l_linger = 0; // Zero timeout - immediate RST
                                   setsockopt(clientfd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
                                   close(clientfd);
                               },
                               "Connection is broken"}),
    [](const testing::TestParamInfo< NotificationTestParams >& info) {
        // Create test name based on notification type
        std::string name = info.param.description;
        // Remove spaces and special characters for a valid test name
        std::replace(name.begin(), name.end(), ' ', '_');
        std::replace(name.begin(), name.end(), '-', '_');
        return name;
    });

// Simple test for initial connection
TEST_F(ListenerTest, ConnectionHandlerReceivesNewConnection) {
    int clientfd = connectClient("127.0.0.3", "12345");
    ASSERT_EQ(connHdlr->_calledWith.size(), 1);
    ASSERT_TRUE(isHandlerCalledWith(portfd, READY_TO_READ));
    close(clientfd);
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
