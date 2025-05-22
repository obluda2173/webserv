#include "HttpParser.h"
#include "test_stubs.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(HttpParserTest, test1) {
    StubLogger logger;
    HttpParser* _prsr = new HttpParser(logger);

    int c = 0;
    while (c++ < 3) {
        std::string _readBuf =
            "GET / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\nCache-Control: "
            "max-age=0\r\nsec-ch-ua: "
            "\"Not(A:Brand\";v=\"24\", \"Chromium\";v=\"122\"\r\nsec-ch-ua-mobile: ?0\r\nsec-ch-ua-platform: "
            "\"Linux\"\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (X11; Linux "
            "x86_64) AppleWebKit/537.36 (KHTML, like Gecko) QtWebEngine/6.8.2 "
            "Chrome/122.0.6261.171 Safari/537.36\r\nAccept: "
            "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/"
            "apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\r\nDNT: 1\r\nAccept-Language: "
            "en-US,en;q=0.9\r\nSec-Fetch-Site: none\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-User: "
            "?1\r\nSec-Fetch-Dest: document\r\nAccept-Encoding: gzip, deflate, br\r\nCookie: "
            "supabase-auth-token=%5B%22eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9."
            "eyJhdWQiOiJhdXRoZW50aWNhdGVkIiwiZXhwIjoxNzE0NTUwMTc4LCJpYXQiOjE3MTQ1NDY1NzgsImlzcyI6Im"
            "h0dHA6Ly8xMjcuMC4wLjE6NTQzMjEvYXV0aC92MSIsInN1YiI6IjM5NDE4ZTNiLTAyNTgtNDQ1Mi1hZjYwLTdh"
            "Y2ZjYzEyNjNmZiIsImVtYWlsIjoiYWRtaW5AcXVpdnIuYXBwIiwicGhvbmUiOiIiLCJhcHBfbWV0YWRhdGEiOn"
            "sicHJvdmlkZXIiOiJlbWFpbCIsInByb3ZpZGVycyI6WyJlbWFpbCJdfSwidXNlcl9tZXRhZGF0YSI6e30sInJv"
            "bGUiOiJhdXRoZW50aWNhdGVkIiwiYWFsIjoiYWFsMSIsImFtciI6W3sibWV0aG9kIjoicGFzc3dvcmQiLCJ0aW"
            "1lc3RhbXAiOjE3MTQ1NDY1Nzh9XSwic2Vzc2lvbl9pZCI6ImFmYjcyM2M1LTAxNTMtNGE2ZS05NWJmLWYyOTcw"
            "NDYyYjMzMiJ9.W4pUmc7LXT_aG37HfYHxJlbKg23gJNUWJXzylwIfxR8%22%2C%"
            "220X19StBUSef0W0PPLb4c5Q%22%2Cnull%2Cnull%2Cnull%5D\r\n\r\n";

        size_t _readBufUsedSize = _readBuf.size();
        const char* b = _readBuf.data();
        size_t count = 0;
        while (count < _readBufUsedSize) {
            _prsr->feed(b, 1);
            if (_prsr->error() || _prsr->ready()) {
                memmove((void*)_readBuf.data(), b + 1, _readBufUsedSize - (count + 1));
                _readBufUsedSize -= (count + 1);
                if (_prsr->ready())
                    _prsr->getRequest();
                break;
            }
            b++;
            count++;
        }
    }
    delete _prsr;
}
