#ifndef TEST_MAIN_H_
#define TEST_MAIN_H_

#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

class MockLogger;

// test_utils.h
void setSvrAddr(sockaddr_in& svrAddr, int port);
int countOpenFileDescriptors();
int newListeningSocket(int port);
int newListeningSocket1(const char* node, const char* port);
int newSocket(const char* node, const char* port);
void getSvrAddrInfo(const char* node, const char* port, struct addrinfo** addrInfo);

// test_server.cpp
void testOneConnectionWithLogging(MockLogger* mLogger, std::string& clientPort, std::string& clientIp,
                                  sockaddr_in svrAddr);
void testMultipleConnectionsWithLogging(MockLogger* mLogger, int port, int nbrConns);
void testOneConnection(std::string& clientPort, std::string& clientIp, sockaddr_in svrAddr);
void testMultipleConnections(int port, int nbrConns);

#endif // TEST_MAIN_H_
