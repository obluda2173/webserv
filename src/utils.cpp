#include "utils.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int new_socket(int port) {
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
