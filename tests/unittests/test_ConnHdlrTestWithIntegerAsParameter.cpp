#include "test_ConnectionHandlerFixture.h"
#include <thread>
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
