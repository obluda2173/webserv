#ifndef UTILS_H
#define UTILS_H

int newListeningSocket(const char* node, const char* port);
void getSvrAddrInfo(const char* node, const char* port, struct addrinfo** addrInfo);
int newSocket(const char* node, const char* port);
#endif // UTILS_H
