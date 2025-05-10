#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "GetHandler.h"
#include "DeleteHandler.h"

class RequestBuilder {
private:
    HttpRequest req;
public:
    RequestBuilder() {
        req.method = "";
        req.uri = "/test.txt";
        req.version = "HTTP/1.1";
        req.headers = {};
    }
    RequestBuilder& withMethod(const std::string& method) {
        req.method = method;
        return *this;
    }
    RequestBuilder& withUri(const std::string& uri) {
        req.uri = uri;
        return *this;
    }
    RequestBuilder& withHeader(const std::string& key, const std::string& value) {
        req.headers[key] = value;
        return *this;
    }
    HttpRequest build() const { return req; }
};

class RouteConfigBuilder {
private:
    RouteConfig cfg;
public:
    RouteConfigBuilder() {
        cfg.root = "./tests/unittests/test_root";
        cfg.index = {};
        cfg.clientMaxBody = 0;
        cfg.autoindex = false;
    }
    RouteConfigBuilder& withRoot(const std::string& root) {
        cfg.root = root;
        return *this;
    }
    RouteConfigBuilder& withAutoIndex(bool autoIndex) {
        cfg.autoindex = autoIndex;
        return *this;
    }
    RouteConfigBuilder& withErrorPage(std::map<int, std::string> errorPage) {
        cfg.errorPage = errorPage;
        return *this;
    }
    RouteConfigBuilder& withIndex(std::vector<std::string> index) {
        cfg.index = index;
        return *this;
    }
    RouteConfig build() const { return cfg; }
};

class ResponseBuilder {
private:
    HttpResponse resp;
public:
    ResponseBuilder() {
        resp.version = "HTTP/1.1";
        resp.statusCode = 400;
        resp.statusMessage = "Bad Request";
        resp.contentType = "";
        resp.contentLanguage = "en-US";
        resp.contentLength = 0;
        resp.body = nullptr;
        resp.isRange = false;
        resp.isClosed = false;
        resp.isChunked = false;
    }
    ResponseBuilder& withStatusCode(int code) {
        resp.statusCode = code;
        return *this;
    }
    ResponseBuilder& withStatusMessage(const std::string& message) {
        resp.statusMessage = message;
        return *this;
    }
    ResponseBuilder& withContentType(const std::string& type) {
        resp.contentType = type;
        return *this;
    }
    ResponseBuilder& withContentLength(int contentLength) {
        resp.contentLength = contentLength;
        return *this;
    }
    HttpResponse build() const { return resp; }
};

inline void assertEqualHttpResponse(const HttpResponse& want, const HttpResponse& got) {
    EXPECT_EQ(want.version, got.version);
    EXPECT_EQ(want.statusCode, got.statusCode);
    EXPECT_EQ(want.statusMessage, got.statusMessage);
    EXPECT_EQ(want.contentType, got.contentType);
    EXPECT_EQ(want.contentLength, got.contentLength);
    if (want.contentLength > 0 && got.body != nullptr) {
        char gotBuffer[2048];
        std::string gotString;
        while (!got.body->isDone() && gotString.size() < static_cast<size_t>(got.contentLength)) {
            size_t bytesWritten = got.body->read(gotBuffer, 2048);
            gotString += std::string(gotBuffer, bytesWritten);
        }
        EXPECT_EQ(want.contentLength, gotString.length());
    }
}

#endif // TEST_UTILS_H
