#include "test_fixtures.h"
#include "test_main.h"
#include "gtest/gtest.h"

TEST_P(ServerWithMockLoggerParametrizedPortTest, connectionTest) {
    std::vector<std::string> listeningPorts = GetParam();
    for (size_t i = 0; i < listeningPorts.size(); i++)
        testMultipleConnectionsWithLogging(_mLogger, listeningPorts[i], 100);
}

INSTANTIATE_TEST_SUITE_P(ServerTests, ServerWithMockLoggerParametrizedPortTest,
                         ::testing::Values(std::vector<std::string>{"8080"}, std::vector<std::string>{"8080", "8081"},
                                           std::vector<std::string>{"8080", "8081", "8082"}));

// // // TODO: make proper integration tests with either golang or python
// // // No parallel stuff in CPP
// TEST_P(ServerTest, connectionTest) {
//     std::vector<int> listeningPorts = GetParam();
//     std::vector<std::thread> threads;
//     for (size_t i = 0; i < listeningPorts.size(); i++)
//         threads.push_back(std::thread(&testMultipleConnections, listeningPorts[i], 10));
//     for (size_t i = 0; i < threads.size(); i++)
//         threads[i].join();
// }

// INSTANTIATE_TEST_SUITE_P(ServerTests, ServerTest,
//                          ::testing::Values(std::vector<int>{8080}, std::vector<int>{8080, 8081},
//                                            std::vector<int>{8080, 8081, 8082}));
