#include "Socket.h"
#include "Logger.h"
#include <iostream>
#include <stdexcept>

int fun(int port) {
    int server_fd;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        return -1;
    }

    // Define a sockaddr_in structure for network address information
    struct sockaddr_in address;

    // Set the address family to IPv4
    address.sin_family = AF_INET;
    // Set the IP address to any available interface
    address.sin_addr.s_addr = INADDR_ANY;
    // Set the port number, converting it to network byte order
    address.sin_port = htons(port);

    // binding the address to the socket file descriptor server_fd
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return -1;
    }

    if (listen(server_fd, 3) < 0) { // TODO: figure out the backlog, that we want to use
        perror("listen");
        return -1;
    }
    return server_fd;
}

Socket::Socket(void) : _port(80) {
    _logger = Logger();
    if ((_server_fd = fun(_port)) == -1) {
        _logger.log("ERROR");
        throw std::runtime_error("");
    }
    _logger.log("INFO");
}

Socket::Socket(int port) : _port(port) {
    _logger = Logger();
    if ((_server_fd = fun(_port)) == -1) {
        _logger.log("ERROR");
        throw std::runtime_error("");
    }
    _logger.log("INFO");
}
