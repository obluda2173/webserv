#include "test_main.h"
#include "MockLogger.h"
#include <gtest/gtest.h>

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

class ServerTest : public ::testing::Test {
  protected:
    MockLogger *mockLogger;

    void SetUp() override { mockLogger = new MockLogger(); }

    void TearDown() override { delete mockLogger; }
};

TEST(FirstTest, test) {
}
