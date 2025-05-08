#include "test_ConnectionHandlerFixture.h"

TEST_P(ConnHdlrTestAsyncMultipleConnectionsReadSizeLimited, sendMsgsAsync) {
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
            readTillNothingMoreToRead(_ioNotifier, _connHdlr, connfd, 2);

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
        verifyThatConnIsSetToREADY_TO_WRITEinsideIIONotifierWithMaxEvents(_ioNotifier, connfd, 2);
        _connHdlr->handleConnection(connfd, READY_TO_WRITE);
        ssize_t r = recv(clientfd, buffer, 1024, 0);
        buffer[r] = '\0';

        EXPECT_STREQ(buffer, wantResponses[i].c_str());
    }
}

INSTANTIATE_TEST_SUITE_P(sendMsgsAsync, ConnHdlrTestAsyncMultipleConnectionsReadSizeLimited,
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
