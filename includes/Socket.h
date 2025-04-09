#ifndef SOCKET_H
#define SOCKET_H

#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <Logger.h>

class Socket {
private:
	int _port;
	int _server_fd;
	Logger _logger;
public:
	Socket(void);
	Socket(int port);
};

#endif // SOCKET_H
