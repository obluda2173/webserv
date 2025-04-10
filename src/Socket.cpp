#include "Socket.h"
#include "Logger.h"

std::string to_string(int value) {
	std::stringstream ss;
	ss << value;
	return ss.str();
}

Server::Server(int port, Logger* logger) : _port(port), _logger(logger) {
	if (_init(_port) == -1) {
		_logger->log("ERROR", "Failed to initialize server on port " + to_string(port));
		throw std::runtime_error("Failed to initialize server");
	}
	_logger->log("INFO", "Server initialized on port " + to_string(port));

	if (_bind() == -1) {
		_logger->log("ERROR", "Failed to bind server on port " + to_string(port));
		throw std::runtime_error("Failed to bind server");
	}
	_logger->log("INFO", "Server successfully bound on port " + to_string(port));
}

Server::~Server() {
	if (_server_fd > 0) {
		close(_server_fd);
	}
}

int Server::_init(int port) {
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		_logger->log("ERROR", "socket: " + std::string(strerror(errno)));
		return -1;
	}

	// set up address information
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(port);

	_server_fd = server_fd;
	_addrlen = sizeof(_address);
	return 0;
}

int Server::_bind() {
	if (bind(_server_fd, (struct sockaddr*)&_address, _addrlen) < 0) {
		_logger->log("ERROR", "bind: " + std::string(strerror(errno)));
		close(_server_fd);
		return -1;
	}
	return 0;
}

int Server::listen() {
	if (::listen(_server_fd, 3) < 0) {
		_logger->log("ERROR", "listen: " + std::string(strerror(errno)));
		close(_server_fd);
		return -1;
	}
	_logger->log("INFO", "Server listening on port " + to_string(_port));
	return 0;
}

int Server::handleConnections() {
	while (true) {
		int new_socket = accept(_server_fd, reinterpret_cast<struct sockaddr*>(&_address), &_addrlen);
		if (new_socket < 0) {
			_logger->log("ERROR", "accept: " + std::string(strerror(errno)));
			break;
		}

		if (processClient(new_socket) < 0) {
			close(new_socket);
			continue;
		}
		close(new_socket);
	}
	close(_server_fd);
	return 0;
}

int Server::processClient(int client_socket) {
	char buffer[BUFFER_SIZE] = {0};
	ssize_t valread = read(client_socket, buffer, BUFFER_SIZE);
	if (valread < 0) {
		_logger->log("ERROR", "read: " + std::string(strerror(errno)));
		return -1;
	}

	_logger->log("INFO", buffer);
	std::string response = generateHttpResponse();
	send(client_socket, response.c_str(), response.size(), 0);
	_logger->log("INFO", "echo message sent");
	return 0;
}

std::string Server::generateHttpResponse() {
	return "HTTP/1.1 200 OK\n"
			"Content-Type: text/html\n"
			"Content-Length: 52\n"
			"\n"
			"<html><body><h1>hello from webserv</h1></body></html>";
}