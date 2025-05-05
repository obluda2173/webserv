#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "GetHandler.h"

struct TestGetHandlerParams {
    Connection conn;
    HttpRequest request;
    RouteConfig config;
    HttpResponse wantResponse;
};


class TestGetHandler : public ::testing::TestWithParam<TestGetHandlerParams> {};

void assertEqualHttpRequest(const ResponseStructure& want, const ResponseStructure& got) {
    EXPECT_EQ(want.version, got.version);
    EXPECT_EQ(want.statusCode, got.statusCode);
    EXPECT_EQ(want.errorMessage, got.errorMessage);
    EXPECT_EQ(want.headers, got.headers);
    EXPECT_EQ(want.body, got.body);
}

TEST_P(TestGetHandler, GetHandler) {
    const auto& params = GetParam();

    // Split request into chunks to simulate partial data
    // HttpResponse gotResponse = GetHandler::handle(params.conn, params.request, params.config)
    GetHandler::handle(params.conn, params.request, params.config)
    HttpResponse gotResponse = params.conn->getResponse();
    assertEqualHttpRequest(params.wantResponse, gotResponse);
}

INSTANTIATE_TEST_SUITE_P(
    SomeRandomName, TestGetHandler,
    ::testing::values(
        // first test
        TestGetHandlerParams{

        }
    )
);