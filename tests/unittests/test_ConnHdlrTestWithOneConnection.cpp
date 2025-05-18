#include "IIONotifier.h"
#include "test_ConnectionHandlerFixture.h"
#include <sys/socket.h>
#include <thread>

TEST_F(ConnHdlrTestWithOneConnectionMockLogger, TestShutdownClosesConnection) {
    int clientfd = _clientFdsAndConnFds[0].first;
    int connfd = _clientFdsAndConnFds[0].second;

    close(clientfd);
    EXPECT_CALL(*_logger, log("INFO", testing::HasSubstr("Disconnect IP")));
    _connHdlr->handleConnection(connfd, READY_TO_READ);
}

TEST_F(ConnHdlrTestWithOneConnection, TestBadRequestClosesConnection) {
    // testing that a bad request is going to close the connection
    int clientfd = _clientFdsAndConnFds[0].first;
    int connfd = _clientFdsAndConnFds[0].second;
    char buffer[1024];
    std::string request = "GET \r\n\r\n";
    std::string wantResponse = "HTTP/1.1 400 Bad Request\r\n"
                               "Content-Length: 0\r\n"
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

    std::vector< std::string > requests = params.requests;
    std::vector< std::string > wantResponses = params.wantResponses;
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
    std::vector< std::string > requests = params.requests;
    std::vector< std::string > wantResponses = params.wantResponses;
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
                                                                         {"HTTP/1.1 400 Bad Request\r\n"
                                                                          "Content-Length: 0\r\n"
                                                                          "\r\n"}},
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
                                                                             "HTTP/1.1 400 Bad Request\r\n"
                                                                             "Content-Length: 0\r\n"
                                                                             "\r\n",
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
                                                                          "Content-Length: 0\r\n"
                                                                          "\r\n"}}));
