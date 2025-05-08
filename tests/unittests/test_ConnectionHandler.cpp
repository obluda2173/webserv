#include "IIONotifier.h"
#include "test_ConnectionHandlerFixture.h"
#include "test_main.h"
#include "utils.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <netdb.h>
#include <stdexcept>
#include <sys/select.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

TEST_F(ConnHdlrTestWithOneConnection, TestBadRequestClosesConnection) {
    // testing that a bad request is going to close the connection
    int clientfd = _clientFdsAndConnFds[0].first;
    int connfd = _clientFdsAndConnFds[0].second;
    char buffer[1024];
    std::string request = "GET \r\n\r\n";
    std::string wantResponse = "HTTP/1.1 400 Bad Request\r\n"
                               "\r\n";

    // Send malformed request
    send(clientfd, request.c_str(), request.length(), 0);

    readUntilREADY_TO_WRITE(_ioNotifier, _connHdlr, connfd);

    // Handle the write event - server should send the 400 response
    _connHdlr->handleConnection(connfd, READY_TO_WRITE);

    // Receive and verify the response
    ssize_t r = recv(clientfd, buffer, 1024, 0);
    EXPECT_NE(r, -1) << "recv: " << std::strerror(errno) << std::endl;
    buffer[r] = '\0';
    EXPECT_STREQ(buffer, wantResponse.c_str());

    // Now verify that the server closed the connection after sending the response
    // Small delay to allow for connection closure
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Check if connection is closed
    ASSERT_FALSE(fcntl(connfd, F_GETFD) != -1 || errno != EBADF) << "Connection is still open";

    close(clientfd);
}

TEST_P(ConnHdlrTestWithOneConnection, TestPersistenceSendInBatches) {
    ParamsVectorRequestsResponses params = GetParam();
    int clientfd = _clientFdsAndConnFds[0].first;
    int connfd = _clientFdsAndConnFds[0].second;

    std::vector<std::string> requests = params.requests;
    std::vector<std::string> wantResponses = params.wantResponses;
    // send msg
    std::string request;
    std::string wantResponse;
    std::string gotResponse;
    int batchSize = 2;
    for (size_t i = 0; i < requests.size(); i++) {
        request = requests[i];
        wantResponse = wantResponses[i];
        std::thread batchSenderThread(
            [request, clientfd, batchSize]() { sendMsgInBatches(request, clientfd, batchSize); });
        batchSenderThread.detach();
        readUntilREADY_TO_WRITE(_ioNotifier, _connHdlr, connfd);
        gotResponse = getResponseConnHdlr(connfd, _connHdlr, clientfd);
        EXPECT_STREQ(wantResponse.c_str(), gotResponse.c_str());
    }

    close(clientfd);
}

TEST_P(ConnHdlrTestWithOneConnection, TestPersistenceSendInOneMsg) {
    ParamsVectorRequestsResponses params = GetParam();
    int clientfd = _clientFdsAndConnFds[0].first;
    int connfd = _clientFdsAndConnFds[0].second;

    char buffer[1024];
    std::vector<std::string> requests = params.requests;
    std::vector<std::string> wantResponses = params.wantResponses;
    // send msg
    for (size_t i = 0; i < requests.size(); i++) {
        std::string request = requests[i];
        std::string wantResponse = wantResponses[i];
        send(clientfd, request.c_str(), request.length(), 0);
        readUntilREADY_TO_WRITE(_ioNotifier, _connHdlr, connfd);

        // verify that the connection in IONotifier is set to READY_TO_WRITE (which the connectionHandler should
        // initiate)
        verifyThatConnIsSetToREADY_TO_WRITEinsideIIONotifier(_ioNotifier, connfd);

        // check that nothing is sent back yet
        recv(clientfd, buffer, 1024, 0);
        ASSERT_EQ(errno, EWOULDBLOCK);

        // next time around the response is sent back
        _connHdlr->handleConnection(connfd, READY_TO_WRITE);
        ssize_t r = recv(clientfd, buffer, 1024, 0);
        buffer[r] = '\0';
        EXPECT_STREQ(buffer, wantResponse.c_str());
    }

    close(clientfd);
}

INSTANTIATE_TEST_SUITE_P(sendMsgsAsync, ConnHdlrTestWithOneConnection,
                         ::testing::Values(ParamsVectorRequestsResponses{{"GET \r\n"},
                                                                         {"HTTP/1.1 400 Bad Request\r\n\r\n"}},
                                           ParamsVectorRequestsResponses{{"GET /ping HTTP/1.1\r\n"
                                                                          "Host: test.com\r\n"
                                                                          "\r\n"},
                                                                         {"HTTP/1.1 200 OK\r\n"
                                                                          "Content-Length: 4\r\n"
                                                                          "\r\n"
                                                                          "pong"}},
                                           ParamsVectorRequestsResponses{{"GET /ping HTTP/1.1\r\n"
                                                                          "Host: test.com\r\n"
                                                                          "\r\n",
                                                                          "GET \r\n"},
                                                                         {
                                                                             "HTTP/1.1 200 OK\r\n"
                                                                             "Content-Length: 4\r\n"
                                                                             "\r\n"
                                                                             "pong",
                                                                             "HTTP/1.1 400 Bad Request\r\n\r\n",
                                                                         }},
                                           ParamsVectorRequestsResponses{{"GET /ping HTTP/1.1\r\n"
                                                                          "Host: test.com\r\n"
                                                                          "\r\n",
                                                                          "GET /ping HTTP/1.1\r\n"
                                                                          "Host: test.com\r\n"
                                                                          "\r\n",
                                                                          "GET \r\n"},
                                                                         {"HTTP/1.1 200 OK\r\n"
                                                                          "Content-Length: 4\r\n"
                                                                          "\r\n"
                                                                          "pong",
                                                                          "HTTP/1.1 200 OK\r\n"
                                                                          "Content-Length: 4\r\n"
                                                                          "\r\n"
                                                                          "pong",
                                                                          "HTTP/1.1 400 Bad Request\r\n"
                                                                          "\r\n"}}));

TEST_P(ConnHdlrTestWithOneConnectionPerRequest, sendMsgsAsync) {
    char buffer[1024];
    struct ParamsVectorRequestsResponses params = GetParam();
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
            int clientfd = _clientFdsAndConnFds[count].first;
            int connfd = _clientFdsAndConnFds[count].second;

            send(clientfd, toBeSent.c_str(), toBeSent.length(), 0);
            // readUntilREADY_TO_WRITE(_ioNotifier, _connHdlr, connfd);
            _connHdlr->handleConnection(connfd, READY_TO_READ);

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
    for (size_t i = 0; i < _clientFdsAndConnFds.size(); i++) {
        int clientfd = _clientFdsAndConnFds[i].first;
        int connfd = _clientFdsAndConnFds[i].second;
        verifyThatConnIsSetToREADY_TO_WRITEinsideIIONotifier(_ioNotifier, connfd);
        _connHdlr->handleConnection(connfd, READY_TO_WRITE);
        ssize_t r = recv(clientfd, buffer, 1024, 0);
        buffer[r] = '\0';

        EXPECT_STREQ(buffer, wantResponses[i].c_str());
    }
}

INSTANTIATE_TEST_SUITE_P(sendMsgsAsync, ConnHdlrTestWithOneConnectionPerRequest,
                         ::testing::Values(ParamsVectorRequestsResponses{
                             {"GET \r\n", "GET /ping HTTP/1.1\r\n"
                                          "Host: test.com\r\n"
                                          "\r\n"}, // I'm sending in batch size of 3, therefore
                                                   // GET \r\n\r\n provokes 2 Bad requests
                             {"HTTP/1.1 400 Bad Request\r\n"
                              "\r\n",

                              "HTTP/1.1 200 OK\r\n"
                              "Content-Length: 4\r\n"
                              "\r\n"
                              "pong"}}));

TEST_P(ConnHdlrTestWithIntegerAsParameter, pingTestInBatches) {
    int batchSize = GetParam();
    int _clientfd = _clientFdsAndConnFds[0].first;
    int _connfd = _clientFdsAndConnFds[0].second;
    std::string msg = "GET /ping HTTP/1.1\r\n"
                      "Host: test.com\r\n"
                      "\r\n";

    // cutting the msg into parts and send
    int clientfd = _clientfd;
    std::thread batchSenderThread([msg, clientfd, batchSize]() { sendMsgInBatches(msg, clientfd, batchSize); });
    batchSenderThread.detach();

    readUntilREADY_TO_WRITE(_ioNotifier, _connHdlr, _connfd);

    std::string gotResponse = getResponseConnHdlr(_connfd, _connHdlr, _clientfd);
    std::string wantResponse = "HTTP/1.1 200 OK\r\n"
                               "Content-Length: 4\r\n"
                               "\r\n"
                               "pong";
    EXPECT_STREQ(gotResponse.c_str(), wantResponse.c_str());
}

TEST_P(ConnHdlrTestWithIntegerAsParameter, multipleRequestsOneConnectionInBatches) {
    int batchSize = GetParam();
    int _clientfd = _clientFdsAndConnFds[0].first;
    int _connfd = _clientFdsAndConnFds[0].second;
    // 2 messages
    std::string msg = "GET /ping HTTP/1.1\r\n"
                      "Host: test.com\r\n"
                      "\r\n"
                      "GET /ping HTTP/1.1\r\n"
                      "Host: test.com\r\n"
                      "\r\n";
    int nbrMsgs = 2;
    std::string wantResponse = "HTTP/1.1 200 OK\r\n"
                               "Content-Length: 4\r\n"
                               "\r\n"
                               "pong";

    // sending the message in batches (inside another thread so there will be multiple reads -> readUntilREADY_TO_WRITE)
    int clientfd = _clientfd; // can not use _clientfd inside a thread (it is data member of the test class)
    std::thread batchSenderThread([msg, clientfd, batchSize]() { sendMsgInBatches(msg, clientfd, batchSize); });

    int count = 0;
    while (count++ < nbrMsgs) {
        readUntilREADY_TO_WRITE(_ioNotifier, _connHdlr, _connfd);
        std::string gotResponse = getResponseConnHdlr(_connfd, _connHdlr, _clientfd);
        ASSERT_STREQ(wantResponse.c_str(), gotResponse.c_str());
    }

    // TODO: if we were to  detach the thread just after launching it, the clientfd might be closed (Teardown) before we
    // got the READY_TO_WRITE notif (getting the CLIENT_HUNG_UP instead).
    // This is for itself a very interesting case, which I would like to write a test for
    // If we join it just after the launch, We would never simulate receiving stuff in batches (since recv()/read() can
    // read more than the actual batch)
    batchSenderThread.join();
}

INSTANTIATE_TEST_SUITE_P(testingBatchSizesSending, ConnHdlrTestWithIntegerAsParameter,
                         ::testing::Values(1, 2, 3, 4, 11, 21, 22, 23)); // these are Fuzzy-tests for the most part

TEST_F(ConnHdlrTestWithMockLoggerIPv6, acceptANewConnection) {
    std::string clientIp = "00:00:00:00:00:00:00:01";
    std::string clientPort = "10001";
    int clientfd = newSocket(clientIp, clientPort, AF_INET6);
    ASSERT_NE(connect(clientfd, _svrAddrInfo->ai_addr, _svrAddrInfo->ai_addrlen), -1)
        << "connect: " << std::strerror(errno) << std::endl;
    EXPECT_CALL(*_logger, log("INFO", "Connection accepted from IP: " + clientIp + ", Port: " + clientPort));
    _connHdlr->handleConnection(_serverfd, READY_TO_READ);
    close(clientfd);
}
