#include "Socket.h"
#include <stdexcept>

Socket::Socket(void) : _port(80) {
    int server_fd;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) != -1) {
        perror("socket failed");
        throw std::runtime_error("");
    }

    // Define a sockaddr_in structure for network address information
    struct sockaddr_in address;

    // Set the address family to IPv4
    address.sin_family = AF_INET;
    // Set the IP address to any available interface
    address.sin_addr.s_addr = INADDR_ANY;
    // Set the port number, converting it to network byte order
    address.sin_port = htons(_port);

    // binding the address to the socket file descriptor server_fd
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        throw std::runtime_error("");
    }

	if (listen(server_fd, 3) < 0) { // TODO: figure out the backlog, that we want to use
		perror("listen");
        throw std::runtime_error("");
	}
}
Socket::Socket(int port) : _port(port) {}
