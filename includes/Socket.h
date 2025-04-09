#ifndef SOCKET_H
#define SOCKET_H

#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <exception>

class Socket {
private:
	int _port;
public:
	Socket(void);
	Socket(int port);
};

#endif // SOCKET_H
