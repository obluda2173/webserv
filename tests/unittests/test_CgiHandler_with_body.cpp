#include "CgiHandler.h"
#include "BodyParser.h"
#include "Connection.h"
#include "HttpRequest.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include <thread>

struct CgiTestParams2 {
    std::string testName;
    std::string scriptName;
    std::string postBody;
    std::string contentType;
    int expectedStatus;
    std::string wantOutput;
};

class CgiPostHandlerTest : public testing::TestWithParam<CgiTestParams2> {
    protected:
        void simulateBodyParsing(Connection* conn, const std::string& body) {
            BodyParser bodyParser;
            conn->_request.headers["content-length"] = std::to_string(body.size());
            conn->_readBuf.assign(body);
            conn->_bodyFinished = false;
            
            while (!conn->_bodyFinished) {
                bodyParser.parse(conn);
            }
        }
};

TEST_P(CgiPostHandlerTest, HandlesPostRequests) {
    const CgiTestParams2& params = GetParam();
    RouteConfig cfg = {
        "tests/unittests/test_cgi/scripts",
        {}, 
        {},
        1000000,
        false,
        {{"php", "/usr/bin/php-cgi"}, {"py", "/usr/bin/python3"}}
    };

    HttpRequest req = HttpRequest();
    req.method = "POST";
    req.uri = "/" + params.scriptName;
    req.headers["content-type"] = params.contentType;
    
    CgiHandler cgiHandler;
    Connection* conn = new Connection({}, -1, "", NULL, NULL);
    conn->setState(Connection::Handling);

    
    simulateBodyParsing(conn, params.postBody);
    
    ASSERT_TRUE(conn->_bodyFinished);
    ASSERT_EQ(conn->_tempBody, params.postBody);

    cgiHandler.handle(conn, req, cfg);

    auto start = std::chrono::steady_clock::now();
    while (conn->getState() != Connection::SendResponse) {
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(5)) {
            FAIL() << "CGI processing timed out";
        }
        
        if (conn->getState() == Connection::Handling || conn->getState() == Connection::HandlingCgi) {
            cgiHandler.handleCgiProcess(conn);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    ASSERT_EQ(conn->_response.statusCode, params.expectedStatus);
    std::string gotOutput = getOutput(conn);
    ASSERT_EQ(gotOutput, params.wantOutput);

    delete conn;
}

INSTANTIATE_TEST_SUITE_P(
    CgiPostTests,
    CgiPostHandlerTest,
    testing::Values(
        CgiTestParams2{
            CgiTestParams2{
                "FormData",
                "form_processor.py",
                "name=kay&hobby=coding",
                "application/x-www-form-urlencoded",
                200,
                "name kay\nhobby coding\n" 
            },
        },
        CgiTestParams2{
            CgiTestParams2{
                "MoreReasonableTest1",
                "hello_process.py",
                "name=Erik&happy=yes",
                "application/x-www-form-urlencoded",
                200,
                "\n<html><body style='text-align:center;'>\n" 
                "<h1 style='color: green;'>GeeksforGeeks</h1>\n" 
                "<h2>Hello, Erik!</h2>\n" 
                "<p>Thank you for using our script.</p>\n" 
                "<p>Yayy! We're happy too! \?\?\?\?</p>\n" 
                "</body></html>\n"
            },
        },
        CgiTestParams2{
            CgiTestParams2{
                "MoreReasonableTest2",
                "hello_process.py",
                "name=Erik&happy=no&sad=yes",
                "application/x-www-form-urlencoded",
                200,
                "\n<html><body style='text-align:center;'>\n" 
                "<h1 style='color: green;'>GeeksforGeeks</h1>\n" 
                "<h2>Hello, Erik!</h2>\n" 
                "<p>Thank you for using our script.</p>\n" 
                "<p>Oh no! Why are you sad? \?\?\?\?</p>\n" 
                "</body></html>\n"
            },
        },
        CgiTestParams2{
            CgiTestParams2{
                "MissingNameParameter",
                "hello_process.py",
                "happy=yes",
                "application/x-www-form-urlencoded",
                200,
                "\n<html><body style='text-align:center;'>\n" 
                "<h1 style='color: green;'>GeeksforGeeks</h1>\n" 
                "<p>Yayy! We're happy too! \?\?\?\?</p>\n" 
                "</body></html>\n"
            }
        }
        // currently not working, asd I don't understand how boyd actually works
        // CgiTestParams2{
        //     CgiTestParams2{
        //         "OversizedBody",
        //         "form_processor.py",
        //         std::string(10'000'000, 'a'),  // 10MB body (adjust to your server limit)
        //         "application/x-www-form-urlencoded",
        //         413,  // Payload Too Large
        //         ""
        //     }
        // }
    ),
    [](const testing::TestParamInfo<CgiTestParams2>& info) {
        return info.param.testName;
    }
);
