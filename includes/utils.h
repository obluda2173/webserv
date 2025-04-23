#ifndef UTILS_H
#define UTILS_H

#include <string>

void getAddrInfoHelper(const char* node, const char* port, int protocol, struct addrinfo** addrInfo);
int newSocket(struct addrinfo* addrInfo);
int newSocket(std::string ip, std::string port, int protocol);
int newListeningSocket(struct addrinfo* addrInfo, int backlog);
int newListeningSocket(std::string ip, std::string port, int protocol, int backlog);

std::string toLower(const std::string& str);

#endif // UTILS_H
