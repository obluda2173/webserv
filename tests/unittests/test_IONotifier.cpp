#include "EpollIONotifier.h"
#include "IIONotifier.h"
#include "test_stubs.h"
#include <cstring>
#include <gtest/gtest.h>
#include <netdb.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <utils.h>

// test a closed connection
TEST(IONotifierTest, DetectsBrokenConnection) {
    StubLogger logger;
    EpollIONotifier ioNotifier(logger);

    // Create a socket pair for testing
    int sockets[2];
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0) << "socketpair failed: " << strerror(errno);

    int serverSocket = sockets[0];
    int clientSocket = sockets[1];

    // Add the server socket to the notifier
    ioNotifier.add(serverSocket);

    // Now close the client side to simulate a broken connection
    close(clientSocket);

    // Wait for the notification
    std::vector< t_notif > notifs = ioNotifier.wait();

    // Check that we got the correct notification
    ASSERT_GT(notifs.size(), 0) << "No events detected";
    EXPECT_EQ(notifs[0].fd, serverSocket);
    EXPECT_EQ(notifs[0].notif, BROKEN_CONNECTION);

    // Clean up
    close(serverSocket);
}

// test a shutdown RDWR connection (it's a fuzzy test to see the behavior on a SHUTDOWN connection)
TEST(IONotifierTest, DetectsBrokenConnection2) {
    StubLogger logger;
    EpollIONotifier ioNotifier(logger);

    // Create a socket pair for testing
    int sockets[2];
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0) << "socketpair failed: " << strerror(errno);

    int serverSocket = sockets[0];
    int clientSocket = sockets[1];

    // Add the server socket to the notifier
    ioNotifier.add(serverSocket);

    // Now close the client side to simulate a broken connection
    shutdown(clientSocket, SHUT_RDWR);

    // Wait for the notification
    std::vector< t_notif > notifs = ioNotifier.wait();

    // Check that we got the correct notification
    ASSERT_GT(notifs.size(), 0) << "No events detected";
    EXPECT_EQ(notifs[0].fd, serverSocket);
    EXPECT_EQ(notifs[0].notif, BROKEN_CONNECTION);

    // Clean up
    close(serverSocket);
}

// test shutdown write
TEST(IONotifierTest, DetectsClientHungUp) {
    StubLogger logger;
    EpollIONotifier ioNotifier(logger);

    // Create a socket pair for testing
    int sockets[2];
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0) << "socketpair failed: " << strerror(errno);

    int serverSocket = sockets[0];
    int clientSocket = sockets[1];

    // Add the server socket to the notifier
    ioNotifier.add(serverSocket);

    // Now close the client side to simulate a broken connection
    shutdown(clientSocket, SHUT_WR);

    // Wait for the notification
    std::vector< t_notif > notifs = ioNotifier.wait();

    // Check that we got the correct notification
    ASSERT_GT(notifs.size(), 0) << "No events detected";
    EXPECT_EQ(notifs[0].fd, serverSocket);
    EXPECT_EQ(notifs[0].notif, CLIENT_HUNG_UP);

    // Clean up
    close(serverSocket);
}
