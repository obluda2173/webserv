#include "Server.h"
#include "Logger.h"

std::string to_string(int value);
std::string generateHttpResponse();

Server::Server(Logger *logger) : _logger(logger) {
    _serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverfd == -1) {
        _logger->log("ERROR", "socket: " + std::string(strerror(errno)));
        throw std::runtime_error("Failed to initialize server");
    }
    _logger->log("INFO", "Created socket for communication");
}

Server::~Server() {
    if (_serverfd > 0)
        close(_serverfd);
}

void Server::listen(int port) {
    _port = port;
    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = INADDR_ANY;
    _address.sin_port = htons(_port);
    _addrlen = sizeof(_address);

    if (bind(_serverfd, (struct sockaddr *)&_address, _addrlen) < 0) {
        _logger->log("ERROR", "bind: " + std::string(strerror(errno)));
        close(_serverfd);
        throw std::runtime_error("Failed to bind server");
    }

    if (::listen(_serverfd, 3) < 0) {
        _logger->log("ERROR", "listen: " + std::string(strerror(errno)));
        close(_serverfd);
        throw std::runtime_error("Failed to listen to server");
    }
    _logger->log("INFO", "Server listening on port " + to_string(_port));
}

int Server::handleConnections() {
    struct pollfd serverPfd = {_serverfd, POLLIN, 0};
    _pfds.push_back(serverPfd); // Initial file descriptor setup to listen for
                                // new connections

    while (true) {
        // wait for events on all fds
        int ready = poll(_pfds.data(), _pfds.size(), -1);
        if (ready < 0) {
            _logger->log("ERROR", "poll: " + std::string(strerror(errno)));
            throw std::runtime_error("poll error");
        }
        _logger->log("INFO", "new connection is ready");

        // iterate through poll vector
        for (std::vector<struct pollfd>::iterator it = _pfds.begin(); it != _pfds.end();) {
            bool removeFd = false;

            // handle incoming data or connections
            if (it->revents & POLLIN) {
                if (it->fd == _serverfd) {
                    // accept new connection
                    int newSocket = accept(_serverfd, reinterpret_cast<struct sockaddr *>(&_address), &_addrlen);
                    if (newSocket < 0) {
                        _logger->log("ERROR", "accept: " + std::string(strerror(errno)));
                    } else {
                        struct pollfd newPfd = {newSocket, POLLIN, 0};
                        _pfds.push_back(newPfd);
                        _logger->log("INFO", "New connection: " + to_string(newSocket));
                    }
                } else {
                    char buffer[BUFFER_SIZE] = {0};
                    ssize_t bytesRead = read(it->fd, buffer, BUFFER_SIZE);
                    if (bytesRead > 0) {
                        _logger->log("INFO", std::string(buffer, bytesRead));
                        it->events = POLLOUT;
                    } else {
                        close(it->fd);
                        removeFd = true;
                    }
                }
            }
            std::cout << "Before the POLLOUT, this is the itorator now: " << it->fd << std::endl;
            // handle sending response
            if (it->revents & POLLOUT) {
                std::string response = generateHttpResponse();
                ssize_t bytesSent = send(it->fd, response.c_str(), response.size(), 0);
                if (bytesSent > 0) {
                    _logger->log("INFO", "response sent successfully");
                } else {
                    _logger->log("ERROR", "send: " + std::string(strerror(errno)));
                }
                close(it->fd);
                removeFd = true;
            }

            // remove closed connections
            if (removeFd) {
                it = _pfds.erase(it);
            } else {
                ++it;
            }
        }
    }
    return 0;
}

std::string to_string(int value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

std::string generateHttpResponse() {
    return "HTTP/1.1 200 OK\n"
           "Content-Type: text/html\n"
           "Content-Length: 52\n"
           "\n"
           "<html><body><h1>hello from webserv</h1></body></html>";
}
