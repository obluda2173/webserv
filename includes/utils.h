#ifndef WEBSERV_H
#define WEBSERV_H

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int new_socket(int port);

#endif // WEBSERV_H
