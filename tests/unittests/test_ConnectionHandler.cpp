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

bool allZero(std::vector<std::string*> msgs) {
    for (std::vector<std::string*>::iterator it = msgs.begin(); it != msgs.end(); it++) {
        if ((*it)->length() > 0)
            return false;
    }
    return true;
}

TEST_F(ConnectionHdlrTest, send2MsgsAsync) {
    char buffer[1024];
    std::string request1 = "GET \r\n\r\n";
    std::string request2 = "GET /ping HTTP/1.1\r\n\r\n";
    std::vector<std::string> wantResponses = {"HTTP/1.1 400 Bad Request\r\n"
                                              "\r\n",
                                              "HTTP/1.1 200 OK\r\n"
                                              "Content-Length: 4\r\n"
                                              "\r\n"
                                              "pong"};

    std::vector<std::string*> msgs = std::vector<std::string*>{&request1, &request2};
    int batchSize = 3;
    while (!allZero(msgs)) {
        int count = 0;
        for (std::vector<std::string*>::iterator it = msgs.begin(); it != msgs.end(); it++) {
            if ((*it)->length() == 0)
                continue;
            std::string toBeSent = (*it)->substr(0, batchSize);
            int clientfd = _clientfdsAndConns[count].first;
            int conn = _clientfdsAndConns[count].second;

            send(clientfd, toBeSent.c_str(), toBeSent.length(), 0);
            _connHdlr->handleConnection(conn, READY_TO_READ);
            recv(clientfd, buffer, 1024, 0);
            ASSERT_EQ(errno, EWOULDBLOCK);

            try {
                *(*it) = (*it)->substr(batchSize);
            } catch (const std::out_of_range&) {
                (*it)->clear();
            }

            count++;
        }
    }
    for (size_t i = 0; i < _clientfdsAndConns.size(); i++) {
        int clientfd = _clientfdsAndConns[i].first;
        int conn = _clientfdsAndConns[i].second;
        _connHdlr->handleConnection(conn, READY_TO_WRITE);
        ssize_t r = recv(clientfd, buffer, 1024, 0);
        buffer[r] = '\0';
        EXPECT_STREQ(buffer, wantResponses[i].c_str());
    }
}

TEST_P(ConnectionHdlrTestWithParamReqResp, sendMsgInOneBatch) {
    reqRespParam params = GetParam();
    char buffer[1024];
    std::string request = params.request;
    std::string wantResponse = params.wantResponse;

    // send msg
    send(_clientfd, request.c_str(), request.length(), 0);
    _connHdlr->handleConnection(_conn, READY_TO_READ);

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
                         ::testing::Values(1, 2, 11, 21, 22, 23));

// TODO: the next two test do change nothing at the current code
// TODO: maybe handle some specific timeout on a connection, probably responsibility of the Listener
// TEST_F(ConnectionHdlrTestWithMockLoggerIPv4, incompleteRequestThenClose) {
//     char buffer[1024];
//     std::string msg = "GET /ping HTT";

//     // send msg
//     send(_clientfd, msg.c_str(), msg.length(), 0);
//     _connHdlr->handleConnection(_conn, READY_TO_READ);

//     // check that nothing is sent back yet
//     recv(_clientfd, buffer, 1024, 0);
//     ASSERT_EQ(errno, EWOULDBLOCK);

//     // now connection is closed
//     EXPECT_CALL(*_logger, log("INFO", testing::HasSubstr("Disconnect IP")));
//     _connHdlr->handleConnection(_conn, CLIENT_HUNG_UP);
// }
