#include "test_fixtures.h"
#include "test_main.h"
#include "gtest/gtest.h"

TEST_P(ServerWithMockLoggerParametrizedPortTest, connectionTest) {
    std::vector<int> listeningPorts = GetParam();
    for (size_t i = 0; i < listeningPorts.size(); i++) {
        testMultipleConnectionsWithLogging(_mLogger, listeningPorts[i]);
    }
}

INSTANTIATE_TEST_SUITE_P(ServerTests, ServerWithMockLoggerParametrizedPortTest,
                         ::testing::Values(std::vector<int>{8080}, std::vector<int>{8080, 8081},
                                           std::vector<int>{8080, 8081, 8082}));

TEST_P(ServerTest, connectionTest) {
    std::vector<int> listeningPorts = GetParam();
    std::vector<std::thread> threads;
    for (size_t i = 0; i < listeningPorts.size(); i++)
        threads.push_back(std::thread(&testMultipleConnections, listeningPorts[i]));
    for (size_t i = 0; i < threads.size(); i++)
        threads[i].join();
}

INSTANTIATE_TEST_SUITE_P(ServerTests, ServerTest,
                         ::testing::Values(std::vector<int>{8080}, std::vector<int>{8080, 8081},
                                           std::vector<int>{8080, 8081, 8082}));
