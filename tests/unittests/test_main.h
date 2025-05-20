#ifndef TEST_MAIN_H_
#define TEST_MAIN_H_

#include "Connection.h"
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

#include "test_mocks.h"
// class MockLogger;

static std::string ROOT = "tests/unittests/test_files/UploadHandler";
static std::string PREFIX = "/uploads/";
// test_utils.h
int countOpenFileDescriptors();

// test_server.cpp
void testOneConnectionWithLogging(testing::NiceMock< MockLogger >* mLogger, std::string& clientPort,
                                  std::string& clientIp, struct addrinfo* svrAddrInfo);
void testMultipleConnectionsWithLogging(testing::NiceMock< MockLogger >* mLogger, std::string svrPort, int nbrConns);
void testOneConnection(std::string& clientIp, std::string& clientPort, struct addrinfo* svrAddrInfo);
void testMultipleConnections(std::string svrPort, int nbrConns);
void sendMsgInBatches(std::string msg, int clientfd, int batchSize);
bool allZero(std::vector< std::string > msgs);

bool checkNotification(IIONotifier* ioNotifier, t_notif wantNotif);

void readUntilREADY_TO_WRITE(IIONotifier* _ioNotifier, IConnectionHandler* _connHdlr, int _conn);
void readTillNothingMoreToRead(IIONotifier* _ioNotifier, IConnectionHandler* _connHdlr, int _conn);
std::string getResponseConnHdlr(int _conn, IConnectionHandler* _connHdlr, int _clientfd);
std::string getRandomString(size_t length);
Router newRouterTest();

// setup conns requests
Connection* setupConnWithContentLength(std::string filename, size_t contentLength);
Connection* setupConnWithoutContentLength(std::string filename);
Connection* setupConnWithTransferEncoding(std::string filename);

// other
std::string getFileContents(const std::string& filename);
void removeFile(std::string filepath);
int getRandomNumber(int min = 10, int max = 50);

// CgiHandlerTest utils
std::string buildUri(std::string script,
                     std::vector< std::pair< std::string, std::vector< std::string > > > queryParams);

std::string getOutput(Connection* conn);
void expectNotif(t_notif notif, int fd, e_notif eNotif);

#endif // TEST_MAIN_H_
