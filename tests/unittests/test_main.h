#ifndef TEST_MAIN_H_
#define TEST_MAIN_H_

#include "IConnectionHandler.h"
#include "Router.h"
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>

class MockLogger;

// test_utils.h
int countOpenFileDescriptors();

// test_server.cpp
void testOneConnectionWithLogging(MockLogger* mLogger, std::string& clientPort, std::string& clientIp,
                                  struct addrinfo* svrAddrInfo);
void testMultipleConnectionsWithLogging(MockLogger* mLogger, std::string svrPort, int nbrConns);
void testOneConnection(std::string& clientIp, std::string& clientPort, struct addrinfo* svrAddrInfo);
void testMultipleConnections(std::string svrPort, int nbrConns);
void sendMsgInBatches(std::string msg, int clientfd, int batchSize);
bool allZero(std::vector<std::string> msgs);

void verifyThatConnIsSetToREADY_TO_WRITEinsideIIONotifier(IIONotifier* ioNotifier, int conn);
void verifyThatConnIsSetToREADY_TO_READinsideIIONotifier(IIONotifier* ioNotifier, int conn);
void readUntilREADY_TO_WRITE(IIONotifier* _ioNotifier, IConnectionHandler* _connHdlr, int _conn);
void readTillNothingMoreToRead(IIONotifier* _ioNotifier, IConnectionHandler* _connHdlr, int _conn);
std::string getResponseConnHdlr(int _conn, IConnectionHandler* _connHdlr, int _clientfd);
Router newRouterTest();

#endif // TEST_MAIN_H_
