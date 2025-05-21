// #include "CgiHandler.h"
// #include "BodyParser.h"
// #include "Connection.h"
// #include "HttpRequest.h"
// #include "test_main.h"
// #include "gtest/gtest.h"
// #include <gtest/gtest.h>
// #include <thread>

// struct CgiTestParams {
//     std::string testName;
//     std::string scriptName;
//     std::string postBody;
//     std::string contentType;
//     int expectedStatus;
//     std::string wantOutput;
// };

// class CgiPostHandlerTest : public testing::TestWithParam<CgiTestParams> {
//     protected:
//         void simulateBodyParsing(Connection* conn, const std::string& body) {
//             BodyParser bodyParser;
//             conn->_request.headers["content-length"] = std::to_string(body.size());
//             conn->_readBuf.assign(body);
//             conn->_bodyFinished = false;
            
//             while (!conn->_bodyFinished) {
//                 bodyParser.parse(conn);
//             }
//         }
// };

// TEST_P(CgiPostHandlerTest, HandlesPostRequests) {
//     const CgiTestParams& params = GetParam();
//     RouteConfig cfg = {
//         "tests/unittests/test_cgi/scripts",
//         {}, 
//         {},
//         1000000,
//         false,
//         {{"php", "/usr/bin/php-cgi"}, {"py", "/usr/bin/python3"}}
//     };

//     HttpRequest req;
//     req.method = "POST";
//     req.uri = "/" + params.scriptName;
//     req.headers["content-type"] = params.contentType;
    
//     CgiHandler cgiHandler;
//     Connection* conn = new Connection({}, -1, "", NULL, NULL);
//     conn->setState(Connection::Handling);

    
//     // Simulate body parsing phase
//     simulateBodyParsing(conn, params.postBody);
    
//     // Verify body parsing completed successfully
//     ASSERT_TRUE(conn->_bodyFinished);
//     ASSERT_EQ(conn->_tempBody, params.postBody);

//     // Execute CGI handling
//     cgiHandler.handle(conn, req, cfg);

//     // Process CGI execution
//     auto start = std::chrono::steady_clock::now();
//     while (conn->getState() != Connection::SendResponse) {
//         if (std::chrono::steady_clock::now() - start > std::chrono::seconds(5)) {
//             FAIL() << "CGI processing timed out";
//         }
//         // std::cout << conn->getState() << "\n";
        
//         if (conn->getState() == Connection::Handling || conn->getState() == Connection::HandlingCgi) {
//             cgiHandler.handleCgiProcess(conn);
//         }
//         std::this_thread::sleep_for(std::chrono::milliseconds(10));
//     }

//     // Validate results
//     ASSERT_EQ(conn->_response.statusCode, params.expectedStatus);
//     ASSERT_EQ(getOutput(conn), params.wantOutput);

//     delete conn;
// }

// INSTANTIATE_TEST_SUITE_P(
//     CgiPostTests,
//     CgiPostHandlerTest,
//     testing::Values(
//         // Basic POST with URL-encoded data
//         CgiTestParams{
//             "FormData",
//             "form_processor.py",
//             "name=kay&hobby=coding",
//             "application/x-www-form-urlencoded",
//             200,
//             "name: kay\nhobby: coding"
//         }
//     ),
//     [](const testing::TestParamInfo<CgiTestParams>& info) {
//         return info.param.testName;
//     }
// );

// // INSTANTIATE_TEST_SUITE_P(
// //     QueryParamTests,
// //     CgiPostHandlerTest,
// //     testing::Values(
// //         CgiTestParams{
// //             "GetWithParams",
// //             "MqueryParams.py",
// //             "",  // Empty body for GET
// //             "",
// //             200,
// //             "name kay\nhobby coding, running\n"
// //         }
// //     ),
// //     [](const testing::TestParamInfo<CgiTestParams>& info) {
// //         return info.param.testName;
// //     }
// // );