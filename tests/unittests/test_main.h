#ifndef TEST_MAIN_H_
#define TEST_MAIN_H_

#include "IConnectionHandler.h"
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
int countOpenFileDescriptors();

// test_server.cpp
void testOneConnectionWithLogging(MockLogger* mLogger, std::string& clientPort, std::string& clientIp,
                                  struct addrinfo* svrAddrInfo);
void testMultipleConnectionsWithLogging(MockLogger* mLogger, std::string svrPort, int nbrConns);
void testOneConnection(std::string& clientIp, std::string& clientPort, struct addrinfo* svrAddrInfo);
void testMultipleConnections(std::string svrPort, int nbrConns);
void sendMsgInChunks(std::string msg, int conn, int clientfd, IConnectionHandler& connHdlr, int chunkSize,
                     char buffer[1024]);

#endif // TEST_MAIN_H_
