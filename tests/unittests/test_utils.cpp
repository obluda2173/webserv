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

int newListeningSocket(int port) {
    int _serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverfd < 0) {
        perror("socket");
        exit(1);
    }

    sockaddr_in svrAddr = {};
    svrAddr.sin_family = AF_INET;
    svrAddr.sin_addr.s_addr = INADDR_ANY;
    svrAddr.sin_port = htons(port);

    int yes = 1;
    if (setsockopt(_serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    if (bind(_serverfd, (sockaddr*)&svrAddr, sizeof(svrAddr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if (listen(_serverfd, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    return _serverfd;
}

void getSvrAddrInfo(const char* node, const char* port, struct addrinfo** addrInfo) {
    int status;
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((status = getaddrinfo(node, port, &hints, addrInfo) == -1))
        std::cerr << "getaddrinfo error: " << gai_strerror(status);
}

int newSocket(const char* node, const char* port) {
    struct addrinfo* addrInfo;
    getSvrAddrInfo(node, port, &addrInfo);

    int socketfd = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    if (socketfd < 0) {
        perror("socket");
        exit(1);
    }
    int yes = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    if (bind(socketfd, addrInfo->ai_addr, addrInfo->ai_addrlen) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(addrInfo);
    return socketfd;
}

int newListeningSocket1(const char* node, const char* port) {
    int socketfd = newSocket(node, port);
    if (listen(socketfd, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    return socketfd;
}
