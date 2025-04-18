#ifndef WEBSERV_H
#define WEBSERV_H

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


typedef enum SocketType {
	LISTENING_SOCKET,
	CLIENT_SOCKET,
} SocketType;

struct ConnectionInfo {
	struct sockaddr_in addr;
	SocketType type;
	int fd;
};

int new_socket(int port);

#endif // WEBSERV_H
