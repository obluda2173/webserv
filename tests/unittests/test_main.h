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
int getClientSocket(const char *ip, int port);
void setSvrAddr(sockaddr_in &svrAddr);
