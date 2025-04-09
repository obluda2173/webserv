#include "Socket.h"
#include "Logger.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unistd.h>

Server::Server(void) : _port(80) {
    _logger = Logger();
    if ((_server_fd = _fun(_port)) == -1) {
        _logger.log("ERROR");
        throw std::runtime_error("");
    }
    _logger.log("INFO");
}

Server::Server(int port) : _port(port) {
    _logger = Logger();
    if ((_server_fd = _fun(_port)) == -1) {
        _logger.log("ERROR");
        throw std::runtime_error("");
    }
}

int Server::listen() {
    if (::listen(_server_fd, 3) < 0) { // TODO: figure out the backlog, that we want to use
        perror("listen");
        close(_server_fd);
        return -1;
    }

    int new_socket;
    while (true) {
        char buffer[BUFFER_SIZE] = {0};
        if ((new_socket = accept(_server_fd, (struct sockaddr*)&_address, &_addrlen)) < 0) {
            perror("accepted");
            close(new_socket);
            close(_server_fd);
            break;
        }

        ssize_t valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread < 0) {
            perror("read");
            close(new_socket);
    close(_server_fd);
            break;
        }
        std::cout << "received: " << buffer << std::endl;

        std::stringstream msg;
        msg << "HTTP/1.1 200 OK" << std::endl
            << "Content-Type: text/html" << std::endl
            << "Content-Length:52" << std::endl
            << std::endl
            << "<html><body><h1>hello from webserv</h1></body></html>";

        send(new_socket, msg.str().c_str(), msg.str().size(), 0);
        std::cout << "echo message sent" << std::endl;
        close(new_socket);
    }

    close(_server_fd);
    return 0;
}

int Server::_fun(int port) {
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

    _server_fd = server_fd;
    _address = address;
    _addrlen = sizeof(_address);

    // binding the address to the socket file descriptor server_fd
    if (bind(server_fd, (struct sockaddr*)&address, _addrlen) < 0) {
        perror("bind failed");
        close(_server_fd);
        return -1;
    }
    return server_fd;
}
