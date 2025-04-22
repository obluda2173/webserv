#ifndef UTILS_H
#define UTILS_H

void getAddrInfoHelper(const char* node, const char* port, int protocol, struct addrinfo** addrInfo);
int newSocket(struct addrinfo* addrInfo);
int newListeningSocket(struct addrinfo* addrInfo, int backlog);
#endif // UTILS_H
