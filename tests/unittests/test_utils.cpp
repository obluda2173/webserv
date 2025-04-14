#include "test_main.h"

int getClientSocket(const char *ip, int port) {
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return -1;
    }

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
