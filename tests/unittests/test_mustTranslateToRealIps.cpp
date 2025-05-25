#include "ConfigStructure.h"
#include "utils.h"
#include <gtest/gtest.h>

TEST(mustTranslateToRealIps, testLocalhost) {
    ServerConfig cfg;
    cfg.listen = {std::pair< std::string, int >{"localhost", 8080}};

    std::vector< ServerConfig > svrCfgs = {cfg};
    mustTranslateToRealIps(svrCfgs);
    EXPECT_EQ(svrCfgs[0].listen.begin()->first, "127.0.0.1");
}
