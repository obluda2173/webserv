#include "test_main.h"
#include "Server.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

class MockLogger : public ILogger {
public:
    MOCK_METHOD(void, log, (const std::string& level, const std::string& msg), (override));
};

// defining a Test Fixture: ServerTest
class ServerTest : public ::testing::Test {};

TEST_F(ServerTest, firstTest) {
    MockLogger mLogger;

    EXPECT_CALL(mLogger, log("INFO", "Server constructed"));
    Server svr(&mLogger);

    EXPECT_FALSE(svr.isRunning());
    svr.start();
    EXPECT_TRUE(svr.isRunning());
}
