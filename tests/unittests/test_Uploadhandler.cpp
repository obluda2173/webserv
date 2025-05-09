#include "Connection.h"
#include "HttpRequest.h"
#include "RouteConfig.h"
#include <gtest/gtest.h>

TEST(TestUploadHandler, firstTest) {

    std::string filename = "example.txt";
    std::string body = "------WebKitFormBoundary7MA4YWxkTrZu0gW"
                       "Content-Disposition: form-data:; name=\"file\"; filename=\"" +
                       filename +
                       "\""
                       "Content-Type: text/plain"
                       "...file contents..."
                       "------WebKitFormBoundary7MA4YWxkTrZu0gW--";

    std::string headerString = "POST /upload HTTP/1.1\r\n"
                               "Host: example.com\r\n"
                               "Content-Length: " +
                               std::to_string(body.length()) +
                               "\r\n"
                               "Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
                               "\r\n";

    Connection* conn = new Connection({}, -1, NULL, NULL);
    HttpRequest req;
    req.method = "POST";
    req.uri = "upload";
    req.version = "HTTP/1.1";
    req.headers["content-length"] = body.length();
    req.headers["content-type"] = "multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW";
    conn->_request = req;
    send(fd, body, body.length());
    conn->fd = fd;
    conn->readIntoBuf();

    // RouteConfig cfg;
    // IHandler* uploadHdlr = new UploadHdlr();
    uploadHdlr->handle(conn, req, cfg);
    // assert that example.txt was created and check that the file content is correct
}
