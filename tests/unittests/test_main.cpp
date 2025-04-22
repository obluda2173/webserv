#include "test_main.h"
#include "gtest/gtest.h"

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    testing::GTEST_FLAG(filter) = "*ConnectionHdlrTest*";
    // testing::GTEST_FLAG(filter) = "*ListenerTestWithMockLogging*";
    // testing::GTEST_FLAG(filter) = "*ListenerTestWo*";
    // testing::GTEST_FLAG(filter) = "*ListenerTest*";
    // testing::GTEST_FLAG(filter) = "ListenerTest.closingAConnection";
    // testing::GTEST_FLAG(filter) = "ListenerTest.closingAConnection:ListenerTest.multiplePortsTestWoLogging";
    // testing::GTEST_FLAG(filter) = "ServerWithMockLoggerParametrizedPortTest";
    // testing::GTEST_FLAG(filter) = "ServerTest*";
    return RUN_ALL_TESTS();
}
