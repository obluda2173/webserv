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
int getClientSocket(std::string ip, int port);
void setSvrAddr(sockaddr_in& svrAddr, int port);
int countOpenFileDescriptors();

// test_server.cpp
void testOneConnection(MockLogger& mLogger, int& clientPort, std::string& clientIp, sockaddr_in svrAddr);
void testMultipleConnections(MockLogger& mLogger, int port);

#endif // TEST_MAIN_H_
