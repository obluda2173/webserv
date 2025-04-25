#include "IIONotifier.h"
#include "test_ConnectionHandlerFixture.h"
#include "test_main.h"
#include "utils.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <netdb.h>
#include <stdexcept>
#include <sys/select.h>
#include <sys/socket.h>
#include <vector>

TEST_F(ConnectionHdlrTestWithMockLoggerIPv6, acceptANewConnection) {
    std::string clientIp = "00:00:00:00:00:00:00:01";
    std::string clientPort = "10001";
    int clientfd = newSocket(clientIp, clientPort, AF_INET6);
    ASSERT_NE(connect(clientfd, _svrAddrInfo->ai_addr, _svrAddrInfo->ai_addrlen), -1)
        << "connect: " << std::strerror(errno) << std::endl;
    EXPECT_CALL(*_logger, log("INFO", "Connection accepted from IP: " + clientIp + ", Port: " + clientPort));
    _connHdlr->handleConnection(_serverfd, READY_TO_READ);
    close(clientfd);
}

TEST_P(ConnectionHdlrTest, sendMsgsAsync) {
    char buffer[1024];
    struct ParamsConnectionHdlrTestAsync params = GetParam();
    std::vector<std::string> requests = params.requests;
    std::vector<std::string> wantResponses = params.wantResponses;

    // sending
    int batchSize = 3;
    while (!allZero(requests)) {
        int count = 0;
        for (std::vector<std::string>::iterator it = requests.begin(); it != requests.end(); it++) {
            if ((*it).length() == 0) {
                count++;
                continue;
            }
            // sent substring
            std::string toBeSent = (*it).substr(0, batchSize);
            int clientfd = _clientfdsAndConns[count].first;
            int conn = _clientfdsAndConns[count].second;

            send(clientfd, toBeSent.c_str(), toBeSent.length(), 0);
            _connHdlr->handleConnection(conn, READY_TO_READ);
            recv(clientfd, buffer, 1024, 0);
            ASSERT_EQ(errno, EWOULDBLOCK);

            // update the string with what is remaining
            try {
                (*it) = (*it).substr(batchSize);
            } catch (const std::out_of_range&) {
                (*it).clear();
            }

            count++;
        }
    }
    // receiving
    for (size_t i = 0; i < _clientfdsAndConns.size(); i++) {
        int clientfd = _clientfdsAndConns[i].first;
        int conn = _clientfdsAndConns[i].second;
        _connHdlr->handleConnection(conn, READY_TO_WRITE);
        ssize_t r = recv(clientfd, buffer, 1024, 0);
        buffer[r] = '\0';

        EXPECT_STREQ(buffer, wantResponses[i].c_str());
    }
}

INSTANTIATE_TEST_SUITE_P(sendMsgsAsync, ConnectionHdlrTest,
                         ::testing::Values(ParamsConnectionHdlrTestAsync{{"GET \r\n\r\n", "GET /ping HTTP/1.1\r\n\r\n"},
                                                                         {"HTTP/1.1 400 Bad Request\r\n"
                                                                          "\r\n",
                                                                          "HTTP/1.1 200 OK\r\n"
                                                                          "Content-Length: 4\r\n"
                                                                          "\r\n"
                                                                          "pong"}}));

TEST_P(ConnectionHdlrTestWithParamReqResp, sendMsgInOneBatch) {
    reqRespParam params = GetParam();
    char buffer[1024];
    std::string request = params.request;
    std::string wantResponse = params.wantResponse;

    // send msg
    send(_clientfd, request.c_str(), request.length(), 0);
    _connHdlr->handleConnection(_conn, READY_TO_READ);

    // verify that the connection in IONotifier is set to READY_TO_WRITE (which the connectionHandler should initiate)
    verifyThatConnIsSetToREADY_TO_WRITEinsideIIONotifier(_ioNotifier, _conn);

    // check that nothing is sent back yet
    recv(_clientfd, buffer, 1024, 0);
    ASSERT_EQ(errno, EWOULDBLOCK);

    // next time around the response is sent back
    _connHdlr->handleConnection(_conn, READY_TO_WRITE);
    ssize_t r = recv(_clientfd, buffer, 1024, 0);
    buffer[r] = '\0';
    EXPECT_STREQ(buffer, wantResponse.c_str());
}

INSTANTIATE_TEST_SUITE_P(testingInOneBatchRequestRespons, ConnectionHdlrTestWithParamReqResp,
                         ::testing::Values(reqRespParam{"GET \r\n\r\n", "HTTP/1.1 400 Bad Request\r\n"
                                                                        "\r\n"},
                                           reqRespParam{"GET /ping HTTP/1.1\r\n\r\n", "HTTP/1.1 200 OK\r\n"
                                                                                      "Content-Length: 4\r\n"
                                                                                      "\r\n"
                                                                                      "pong"}));

TEST_P(ConnectionHdlrTestWithParamInt, pingTestInBatches) {
    int batchSize = GetParam();
    char buffer[1024];
    std::string msg = "GET /ping HTTP/1.1\r\n\r\n";

    // cutting the msg into parts and send
    sendMsgInBatches(msg, _conn, _clientfd, *_connHdlr, batchSize, buffer);

    // verify that the connection in IONotifier is set to READY_TO_WRITE (which the connectionHandler should initiate)
    verifyThatConnIsSetToREADY_TO_WRITEinsideIIONotifier(_ioNotifier, _conn);

    // handle teh
    _connHdlr->handleConnection(_conn, READY_TO_WRITE);
    ssize_t r = recv(_clientfd, buffer, 1024, 0);
    buffer[r] = '\0';
    std::string wantResponse = "HTTP/1.1 200 OK\r\n"
                               "Content-Length: 4\r\n"
                               "\r\n"
                               "pong";
    EXPECT_STREQ(buffer, wantResponse.c_str());
}

INSTANTIATE_TEST_SUITE_P(testingBatchSizesSending, ConnectionHdlrTestWithParamInt,
                         ::testing::Values(1, 2, 11, 21, 22, 23)); // these are Fuzzy-tests for the most part
