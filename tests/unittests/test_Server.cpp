#include "test_ServerFixtures.h"
#include "test_main.h"
#include "gtest/gtest.h"

TEST_P(ServerWithMockLoggerParametrizedPortTest, connectionTest) {
    using ::testing::_;
    EXPECT_CALL(*_logger, log(_, _)).Times(testing::AnyNumber());
    for (std::set< std::pair< std::string, std::string > >::iterator it = _addrPorts.begin(); it != _addrPorts.end();
         it++)
        testMultipleConnections(it->second, 10);
}

INSTANTIATE_TEST_SUITE_P(ServerTests, ServerWithMockLoggerParametrizedPortTest,
                         ::testing::Values(std::set< std::pair< std::string, std::string > >{{"0.0.0.0", "8080"}},
                                           std::set< std::pair< std::string, std::string > >{{"0.0.0.0", "8080"},
                                                                                             {"0.0.0.0", "8081"}},
                                           std::set< std::pair< std::string, std::string > >{
                                               {"0.0.0.0", "8080"}, {"0.0.0.0", "8081"}, {"0.0.0.0", "8082"}}));

TEST_P(ServerTestWoMockLogging, connectionTest) {
    for (std::set< std::pair< std::string, std::string > >::iterator it = _addrPorts.begin(); it != _addrPorts.end();
         it++)
        testMultipleConnections(it->second, 10);
}

INSTANTIATE_TEST_SUITE_P(ServerTests, ServerTestWoMockLogging,
                         ::testing::Values(std::set< std::pair< std::string, std::string > >{{"0.0.0.0", "8080"}},
                                           std::set< std::pair< std::string, std::string > >{{"0.0.0.0", "8080"},
                                                                                             {"0.0.0.0", "8081"}},
                                           std::set< std::pair< std::string, std::string > >{
                                               {"0.0.0.0", "8080"}, {"0.0.0.0", "8081"}, {"0.0.0.0", "8082"}}));
