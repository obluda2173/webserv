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
#include <sys/socket.h>

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

TEST_F(ConnectionHdlrTest, send2MsgsParallel) {
    char buffer[1024];
    int clientfd1 = _clientfds[0];
    int conn1 = _conns[0];
    int clientfd2 = _clientfds[1];
    int conn2 = _conns[1];

    std::string request1 = "GET \r\n\r\n";
    std::string request2 = "GET /ping HTTP/1.1\r\n\r\n";
    std::string wantResponse1 = "HTTP/1.1 400 Bad Request\r\n"
                                "\r\n";
    std::string wantResponse2 = "HTTP/1.1 200 OK\r\n"
                                "Content-Length: 4\r\n"
                                "\r\n"
                                "pong";

    // send first half of request1
    send(clientfd1, request1.substr(0, 3).c_str(), request1.substr(0, 3).length(), 0);
    _connHdlr->handleConnection(conn1, READY_TO_READ);
    recv(clientfd1, buffer, 1024, 0);
    ASSERT_EQ(errno, EWOULDBLOCK);

    // send first half of request2
    send(clientfd2, request2.substr(0, 4).c_str(), request2.substr(0, 4).length(), 0);
    _connHdlr->handleConnection(conn2, READY_TO_READ);
    recv(clientfd2, buffer, 1024, 0);
    ASSERT_EQ(errno, EWOULDBLOCK);

    // send second half of request1
    send(clientfd1, request1.substr(3).c_str(), request1.substr(3).length(), 0);
    _connHdlr->handleConnection(conn1, READY_TO_READ);
    recv(clientfd1, buffer, 1024, 0);
    ASSERT_EQ(errno, EWOULDBLOCK);

    // send second half of request2
    send(clientfd2, request2.substr(4).c_str(), request2.substr(4).length(), 0);
    _connHdlr->handleConnection(conn2, READY_TO_READ);
    recv(clientfd2, buffer, 1024, 0);
    ASSERT_EQ(errno, EWOULDBLOCK);

    // check that it received the right responses (response1)
    _connHdlr->handleConnection(conn1, READY_TO_WRITE);
    ssize_t r = recv(clientfd1, buffer, 1024, 0);
    buffer[r] = '\0';
    EXPECT_STREQ(buffer, wantResponse1.c_str());

    // check that it received the right responses (response2)
    _connHdlr->handleConnection(conn2, READY_TO_WRITE);
    r = recv(clientfd2, buffer, 1024, 0);
    buffer[r] = '\0';
    EXPECT_STREQ(buffer, wantResponse2.c_str());

    close(clientfd2);
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
