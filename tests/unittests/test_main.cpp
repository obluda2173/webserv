#include "test_main.h"
#include "gtest/gtest.h"

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    // Don't run the following tests
    // testing::GTEST_FLAG(filter) = "-*ResponseWriterTest*";
    // testing::GTEST_FLAG(filter) = "-*TestHttpParser*";
    // testing::GTEST_FLAG(filter) = "-*ListenerTest*:*ServerTest*";
    // testing::GTEST_FLAG(filter) = "-*ServerTest*";
    // testing::GTEST_FLAG(filter) = "-*ConnHdlrTestOneConnection.TestPersistenceSendInBatches*";
    // testing::GTEST_FLAG(filter) = "-*sendMsgsAsync/ConnectionHdlrTestOneConnection.TestPersistenceSendInBatches*";
    // testing::GTEST_FLAG(filter) = "-*ConnHdlrTestWithParamInt.multipleRequestsOneConnectionInBatches*";
    // testing::GTEST_FLAG(filter) = "-*TestHttpParser*";
    //
    // only run the following test

    testing::GTEST_FLAG(filter) = "*ConnHdlrTestWithBigBody*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestWithOneConnectionPerRequest.sendMsgsAsync*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestWithOneConnection.TestPersistenceSendInBatches*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestWithOneConnection.TestBadRequestClosesConnection*";
    // testing::GTEST_FLAG(filter) = "*ConnectionHdlrTestWithParamInt.multipleRequestsOneConnectionInBatches*";
    // testing::GTEST_FLAG(filter) = "*ResponseWriterTest*";
    // testing::GTEST_FLAG(filter) = "*testingBatchSizesSending*";
    // testing::GTEST_FLAG(filter) = "*pingTestInBatches*";
    // testing::GTEST_FLAG(filter) = "*multipleRequestsOneConnectionInBatches*";
    // testing::GTEST_FLAG(filter) = "*RouterTest*";
    // testing::GTEST_FLAG(filter) = "*IONotifierTest*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTest.send2MsgsAsync*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestWithParamInt.multipleRequestsOneConnectionInBatches*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestOneConnection.TestBadRequestClosesConnection*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestOneConnection.TestPersistenceSendInBatches*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestWithOneConnection.TestPersistenceSendInOneMsg*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestWithParamInt.multipleRequestsOneConnectionInBatches*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTest*";
    // testing::GTEST_FLAG(filter) = "*ListenerTestWithMockLogging*";
    // testing::GTEST_FLAG(filter) = "*ListenerTestWithMockLogging.closing*";
    // testing::GTEST_FLAG(filter) = "*ListenerTestWo*";
    // testing::GTEST_FLAG(filter) = "*ListenerTest*";
    // testing::GTEST_FLAG(filter) = "ListenerTest.closingAConnection";
    // testing::GTEST_FLAG(filter) = "ListenerTest.closingAConnection:ListenerTest.multiplePortsTestWoLogging";
    // testing::GTEST_FLAG(filter) = "ServerWithMockLoggerParametrizedPortTest";
    // testing::GTEST_FLAG(filter) = "ServerTest*";
    // testing::GTEST_FLAG(filter) = "*HttpParser*";
    return RUN_ALL_TESTS();
}
