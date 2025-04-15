#include "test_main.h"
#include <cstring>

void reuseSocket(int socketfd) {
    int yes = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
}

int getClientSocket(const char *ip, int port) {
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return -1;
    }
    reuseSocket(clientfd);

    struct sockaddr_in local_address;
    socklen_t local_address_length = sizeof(local_address);
    local_address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &local_address.sin_addr);
    local_address.sin_port = htons(port);

    if (bind(clientfd, (sockaddr *)&local_address, local_address_length) == -1) {
        std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
        return -1;
    }
    return clientfd;
}

void setSvrAddr(sockaddr_in &svrAddr) {
    svrAddr.sin_family = AF_INET;
    svrAddr.sin_port = htons(8080);
    ASSERT_GT(inet_pton(AF_INET, "127.0.0.1", &(svrAddr.sin_addr)), 0) << "inet_pton: " << strerror(errno);
}
