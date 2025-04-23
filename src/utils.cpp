#include <arpa/inet.h>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void getAddrInfoHelper(const char* node, const char* port, int protocol, struct addrinfo** addrInfo) {
    int status;
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = protocol;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((status = getaddrinfo(node, port, &hints, addrInfo) == -1))
        std::cerr << "getaddrinfo error: " << gai_strerror(status);
}

int newSocket(addrinfo* addrInfo) {
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
    return socketfd;
}

int newSocket(std::string ip, std::string port, int protocol) {
    struct addrinfo* addrInfo;
    getAddrInfoHelper(ip.c_str(), port.c_str(), protocol, &addrInfo);
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

int newListeningSocket(addrinfo* addrInfo, int backlog) {
    int socketfd = newSocket(addrInfo);
    if (listen(socketfd, backlog) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    return socketfd;
}

int newListeningSocket(std::string ip, std::string port, int protocol, int backlog) {
    int socketfd = newSocket(ip, port, protocol);
    if (listen(socketfd, backlog) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    return socketfd;
}

std::string toLower(const std::string& str) {
    std::string result = str;
    for (char* ptr = &result[0]; ptr < &result[0] + result.size(); ++ptr) {
        *ptr = static_cast<char>(tolower(*ptr));
    }
    return result;
}
