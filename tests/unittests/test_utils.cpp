#include "IConnectionHandler.h"
#include "IIONotifier.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <dirent.h>
#include <iostream>
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

void sendMsgInBatches(std::string msg, int clientfd, int batchSize) {
    // cutting the msg into parts and send
    std::vector<std::string> chunks;
    for (std::size_t i = 0; i < msg.length(); i += batchSize) {
        send(clientfd, msg.substr(i, batchSize).c_str(), msg.substr(i, batchSize).length(), 0);
    }
}

bool allZero(std::vector<std::string> msgs) {
    for (std::vector<std::string>::iterator it = msgs.begin(); it != msgs.end(); it++) {
        if ((*it).length() > 0)
            return false;
    }
    return true;
}

void verifyThatConnIsSetToREADY_TO_WRITEinsideIIONotifierWithMaxEvents(IIONotifier* ioNotifier, int conn,
                                                                       int maxEvents) {
    int* fds = new int[maxEvents]();
    e_notif* notifs = new e_notif[maxEvents](); // uninitialized

    ioNotifier->wait(fds, notifs);
    int fd = -1;
    e_notif notif;
    for (int i = 0; i < maxEvents; i++) {
        if (fds[i] == conn) {
            fd = fds[i];
            notif = notifs[i];
        }
    }
    delete[] fds;
    delete[] notifs;
    ASSERT_EQ(fd, conn);
    ASSERT_EQ(notif, READY_TO_WRITE);
}

void verifyThatConnIsSetToREADY_TO_WRITEinsideIIONotifier(IIONotifier* ioNotifier, int conn) {
    // verify that the connection in IONotifier is set to READY_TO_WRITE (which the connectionHandler should initiate)
    int fds;
    e_notif notif;
    ioNotifier->wait(&fds, &notif);
    ASSERT_EQ(fds, conn);
    ASSERT_EQ(notif, READY_TO_WRITE);
}

void verifyThatConnIsSetToREADY_TO_READinsideIIONotifier(IIONotifier* ioNotifier, int conn) {
    // verify that the connection in IONotifier is set to READY_TO_WRITE (which the connectionHandler should initiate)
    int fds;
    e_notif notif;
    ioNotifier->wait(&fds, &notif);
    ASSERT_EQ(fds, conn);
    ASSERT_EQ(notif, READY_TO_READ);
}

void readTillNothingMoreToRead(IIONotifier* _ioNotifier, IConnectionHandler* _connHdlr, int _conn, int maxEvents) {
    int* fds = new int[maxEvents]();
    e_notif* notifs = new e_notif[maxEvents](); // uninitialized

    _ioNotifier->wait(fds, notifs);
    int fd = -1;
    e_notif notif;
    for (int i = 0; i < maxEvents; i++) {
        if (fds[i] == _conn) {
            fd = fds[i];
            notif = notifs[i];
        }
    }
    delete[] fds;
    delete[] notifs;

    while (notif == READY_TO_READ && fd != -1) {
        _connHdlr->handleConnection(_conn, READY_TO_READ);

        fds = new int[maxEvents]();
        notifs = new e_notif[maxEvents](); // uninitialized
        _ioNotifier->wait(fds, notifs);
        fd = -1;
        for (int i = 0; i < maxEvents; i++) {
            if (fds[i] == _conn) {
                fd = fds[i];
                notif = notifs[i];
            }
        }
        delete[] fds;
        delete[] notifs;
    }
}

void readUntilREADY_TO_WRITE(IIONotifier* _ioNotifier, IConnectionHandler* _connHdlr, int _conn) {
    int fds = -1;
    e_notif notif;
    _ioNotifier->wait(&fds, &notif);
    while (notif == READY_TO_READ) {
        _connHdlr->handleConnection(_conn, READY_TO_READ);
        _ioNotifier->wait(&fds, &notif);
    }
    EXPECT_NE(fds, -1);
    EXPECT_EQ(notif, READY_TO_WRITE);
}

std::string getResponseConnHdlr(int _conn, IConnectionHandler* _connHdlr, int _clientfd) {
    char buffer[1024];
    ssize_t r;
    _connHdlr->handleConnection(_conn, READY_TO_WRITE);
    r = recv(_clientfd, buffer, 1024, 0);
    if (r == -1) {
        std::cout << "recv: " << std::strerror(errno) << std::endl;
        exit(1);
    }
    buffer[r] = '\0';
    return buffer;
}
