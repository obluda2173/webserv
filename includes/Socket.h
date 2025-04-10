#ifndef SOCKET_H
#define SOCKET_H

#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <Logger.h>
#include <stdexcept>
#include <netinet/in.h>

class Logger;

class Server {
	private:
		struct sockaddr_in _address;
		socklen_t _addrlen;
		int _port;
		int _server_fd;
		Logger* _logger;

		int _init(int port);
		int _bind();

	public:
		static const int BUFFER_SIZE = 1024;

		Server(int port, Logger* logger);
		~Server();

		int listen();
		int handleConnections();
		int processClient(int client_socket);
		std::string generateHttpResponse();
};

#endif // SOCKET_H
