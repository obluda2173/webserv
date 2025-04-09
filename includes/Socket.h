#ifndef SOCKET_H
#define SOCKET_H

#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <Logger.h>

#define BUFFER_SIZE 1024

class Server {
private:
	struct sockaddr_in _address;
	socklen_t _addrlen;
	int _port;
	int _server_fd;
	Logger _logger;
	int _init(int);
	int _bind();
public:
	Server(void);
	Server(int port);
	~Server();
	int listen(void);
};

#endif // SOCKET_H
