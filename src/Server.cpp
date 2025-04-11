#include "Server.h"
#include "Logger.h"

std::string to_string(int value) {
	std::stringstream ss;
	ss << value;
	return ss.str();
}

Server::Server(Logger* logger) : _logger(logger) {
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

	if (bind(_serverfd, (struct sockaddr*)&_address, _addrlen) < 0) {
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
	while (true) {
		int newConn = accept(_serverfd, reinterpret_cast<struct sockaddr*>(&_address), &_addrlen);
		if (newConn < 0) {
			_logger->log("ERROR", "accept: " + std::string(strerror(errno)));
			close(_serverfd);
			throw std::runtime_error("error getting establishing new connection");
			break;
		}

		if (processConn(newConn) < 0) {
			close(_serverfd);
			close(newConn);
			throw std::runtime_error("error processing the connection");
			continue;
		}
		close(newConn);
	}
	close(_serverfd);
	return 0;
}

int Server::processConn(int conn) {
	char buffer[BUFFER_SIZE] = {0};
	ssize_t valread = read(conn, buffer, BUFFER_SIZE);
	if (valread < 0) {
		_logger->log("ERROR", "read: " + std::string(strerror(errno)));
		return -1;
	}

	_logger->log("INFO", buffer);
	std::string response = generateHttpResponse();
	send(conn, response.c_str(), response.size(), 0);
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
