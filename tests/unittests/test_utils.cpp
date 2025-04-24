#include "IConnectionHandler.h"
#include "IIONotifier.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <dirent.h>
#include <iostream>
#include <thread>
#include <unistd.h>

void reuseSocket(int socketfd) {
    int yes = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        std::cerr << "Failed to reuse socket: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

void setAddr(std::string ip, int port, struct addrinfo& hints, struct addrinfo** res) {
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // Use IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

    // Convert port to a string
    std::string port_str = std::to_string(port);

    int status = getaddrinfo(ip.c_str(), port_str.c_str(), &hints, res);
    if (status != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        exit(1);
    }
}

void setSvrAddr(sockaddr_in& svrAddr, int port) {
    svrAddr.sin_family = AF_INET;
    svrAddr.sin_port = htons(port);
    ASSERT_GT(inet_pton(AF_INET, "127.0.0.1", &(svrAddr.sin_addr)), 0) << "inet_pton: " << strerror(errno);
}

int countOpenFileDescriptors() {
    int count = 0;
    DIR* dir = opendir("/proc/self/fd");
    if (dir != nullptr) {
        while (readdir(dir) != nullptr) {
            count++;
        }
        closedir(dir);
        // Adjust count to exclude ".", ".." and the dir fd itself
        count -= 3;
    }
    return count;
}

void sendMsgInBatches(std::string msg, int conn, int clientfd, IConnectionHandler& connHdlr, int batchSize,
                      char buffer[1024]) {
    // cutting the msg into parts and send
    std::vector<std::string> chunks;
    for (std::size_t i = 0; i < msg.length(); i += batchSize) {
        send(clientfd, msg.substr(i, batchSize).c_str(), msg.substr(i, batchSize).length(), 0);
        connHdlr.handleConnection(conn, READY_TO_READ);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        recv(clientfd, buffer, 1024, 0);
        ASSERT_EQ(errno, EWOULDBLOCK);
    }
}

bool allZero(std::vector<std::string> msgs) {
    for (std::vector<std::string>::iterator it = msgs.begin(); it != msgs.end(); it++) {
        if ((*it).length() > 0)
            return false;
    }
    return true;
}

void verifyThatConnIsSetToREADY_TO_WRITEinsideIIONotifier(IIONotifier* ioNotifier, int conn) {
    // verify that the connection in IONotifier is set to READY_TO_WRITE (which the connectionHandler should initiate)
    int fds;
    e_notif notif;
    ioNotifier->wait(&fds, &notif);
    ASSERT_EQ(fds, conn);
    ASSERT_EQ(notif, READY_TO_WRITE);
}
