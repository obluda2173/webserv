#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>

// test_utils.h
int getClientSocket(std::string ip, int port);
void setSvrAddr(sockaddr_in &svrAddr);

// test_server.cpp
class MockLogger;
void testOneConnection(MockLogger &mLogger, int &clientPort, std::string &clientIp, sockaddr_in svrAddr);
void testMultipleConnections(MockLogger &mLogger);
