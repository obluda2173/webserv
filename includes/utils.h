#ifndef UTILS_H
#define UTILS_H

int newListeningSocket(const char* node, const char* port, int protocol);
void getSvrAddrInfo(const char* node, const char* port, int protocol, struct addrinfo** addrInfo);
int newSocket(const char* node, const char* port, int protocol);
#endif // UTILS_H
