#include "Logger.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <sstream>

// Using a test fixture to capture output from std::cout
class LoggerTest : public ::testing::Test {
protected:
    std::streambuf* orig_buf;
    std::ostringstream captured;

    void SetUp() override {
        orig_buf = std::cout.rdbuf();
        std::cout.rdbuf(captured.rdbuf());
    }

    void TearDown() override {
        std::cout.rdbuf(orig_buf);
    }
};

TEST_F(LoggerTest, invalidLevelLogsDefaultMessage) {
    Logger logger;
    logger.log("NOTALEVEL", "Invalid level test");

    std::string output = captured.str();
    EXPECT_THAT(output, testing::HasSubstr("Probably complaining about insignificant problems"));
}

TEST_F(LoggerTest, logFromWithWarningLogsWarningAndError) {
    Logger logger;
    captured.str("");  // clear the capture
    logger.log_from("WARNING", "Test log_from");

    std::string output = captured.str();
    EXPECT_THAT(output, testing::HasSubstr("WARNING Test log_from"));
    EXPECT_THAT(output, testing::HasSubstr("ERROR Test log_from"));
}
