#include "test_main.h"
#include "gtest/gtest.h"

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    testing::GTEST_FLAG(filter) = "-*TestHttpParser*";
    // testing::GTEST_FLAG(filter) = "*IONotifierTest*";
    // testing::GTEST_FLAG(filter) = "*ConnectionHdlrTest.send2MsgsAsync*";

    // testing::GTEST_FLAG(filter) = "*ConnectionHdlrTestOneConnection.TestBadRequestClosesConnection*";
    // testing::GTEST_FLAG(filter) = "*ConnectionHdlrTestOneConnection.TestPersistenceSendInBatches*";
    // testing::GTEST_FLAG(filter) = "*ConnectionHdlrTestOneConnection.TestPersistenceSendInOneMsg*";
    // testing::GTEST_FLAG(filter) = "*ConnectionHdlrTestWithParamInt.multipleRequestsOneConnectionInBatches*";
    // testing::GTEST_FLAG(filter) = "*ConnectionHdlrTest*";
    // testing::GTEST_FLAG(filter) = "*ListenerTestWithMockLogging*";
    // testing::GTEST_FLAG(filter) = "*ListenerTestWithMockLogging.closing*";
    // testing::GTEST_FLAG(filter) = "*ListenerTestWo*";
    // testing::GTEST_FLAG(filter) = "*ListenerTest*";
    // testing::GTEST_FLAG(filter) = "ListenerTest.closingAConnection";
    // testing::GTEST_FLAG(filter) = "ListenerTest.closingAConnection:ListenerTest.multiplePortsTestWoLogging";
    // testing::GTEST_FLAG(filter) = "ServerWithMockLoggerParametrizedPortTest";
    // testing::GTEST_FLAG(filter) = "ServerTest*";
    return RUN_ALL_TESTS();
}
