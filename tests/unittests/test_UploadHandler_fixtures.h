#ifndef TEST_UPLOADHANDLER_FIXTURES_H
#define TEST_UPLOADHANDLER_FIXTURES_H

#include "Connection.h"
#include "test_UploadHandler_utils.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <gtest/gtest.h>

struct UploadHandlerTestParams {
    std::vector< std::string > filenames;
    std::vector< size_t > readBufsLengths;
    std::vector< size_t > bodyLengths;
    size_t batchSize; // amount of bites calling the upload handler handler function
    size_t clientMaxBody;
};

class UploadHdlrTest : public testing::TestWithParam< UploadHandlerTestParams > {
  protected:
    std::vector< std::string > filenames;
    std::vector< size_t > readBufsLengths;
    std::vector< size_t > contentLengths;
    std::vector< std::string > readBufs;
    std::vector< std::string > bodies;
    std::vector< Connection* > conns;
    size_t batchSize;
    size_t clientMaxBody;

  public:
    virtual void SetUp() override {
        UploadHandlerTestParams params = GetParam();
        batchSize = params.batchSize;
        clientMaxBody = params.clientMaxBody;

        filenames = params.filenames;
        readBufsLengths = params.readBufsLengths;
        contentLengths = params.bodyLengths;
        for (size_t i = 0; i < params.readBufsLengths.size(); i++) {
            readBufs.push_back(getRandomString(params.readBufsLengths[i]));
            bodies.push_back(readBufs[i].substr(0, params.bodyLengths[i]));
            conns.push_back(setupConnWithContentLength(params.filenames[i], bodies[i].length()));
        }
    }
};

#endif // TEST_UPLOADHANDLER_FIXTURES_H
