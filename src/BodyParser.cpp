#include "BodyParser.h"
#include "Connection.h"
#include "handlerUtils.h"
#include <sstream>
#include <stdexcept>
#include <string>

#include <cerrno>    // for errno
#include <cstdlib>   // for strtoll
#include <stdexcept> // for exceptions
#include <string>

BodyParser::BodyParser() : _transferEncodingState("readingChunkSize") {}

long long custom_stoll(const std::string& str, size_t* idx = 0, int base = 10) {
    char* end;

    // Reset errno before the conversion
    errno = 0;

    const char* start = str.c_str();
    long long result = strtoll(start, &end, base);

    // Calculate position where parsing stopped (if needed)
    if (idx)
        *idx = end - start;

    // Handle errors like C++11's stoll
    if (errno == ERANGE)
        throw std::out_of_range("stoll: out of range");
    if (end == start)
        throw std::invalid_argument("stoll: invalid argument");

    return result;
}

bool BodyParser::_checkContentLength(Connection* conn, BodyContext& bodyCtx) {
    if (conn->_request.headers.find("content-length") == conn->_request.headers.end()) {
        conn->_bodyFinished = true;
        return false;
    }

    std::stringstream ss(conn->_request.headers["content-length"]);
    ss >> bodyCtx.contentLength;
    if (bodyCtx.contentLength == 0) {
        conn->_bodyFinished = true;
        return false;
    }

    if (bodyCtx.contentLength > conn->route.cfg.clientMaxBody) {
        setErrorResponse(conn->_response, 413, "Content Too Large", conn->route.cfg);
        conn->setState(Connection::SendResponse);
        return false;
    }
    return true;
}

void BodyParser::_parseContentLength(Connection* conn) {
    BodyContext& bodyCtx = conn->bodyCtx;
    Buffer& readBuf = conn->_readBuf;

    if ((bodyCtx.bytesReceived + readBuf.size()) < bodyCtx.contentLength) {
        conn->_tempBody = std::string(conn->_readBuf.data(), conn->_readBuf.size());
        bodyCtx.bytesReceived += conn->_tempBody.size();
        readBuf.clear();
    } else {
        int countRest = bodyCtx.contentLength - bodyCtx.bytesReceived;
        conn->_tempBody = std::string(readBuf.data(), bodyCtx.contentLength - bodyCtx.bytesReceived);
        bodyCtx.bytesReceived = bodyCtx.contentLength;
        conn->_bodyFinished = true;
        readBuf.advance(countRest);
    }
}

long long BodyParser::_validateHex(std::string readBufStr, Connection* conn) {
    long long r;
    try {
        r = custom_stoll(readBufStr, nullptr, 16);
    } catch (const std::invalid_argument& e) {
        conn->_bodyFinished = true;
        conn->setState(Connection::SendResponse);
        setErrorResponse(conn->_response, 404, "Bad Request", conn->route.cfg);
        return -1;
    } catch (const std::out_of_range& e) {
        conn->_bodyFinished = true;
        conn->setState(Connection::SendResponse);
        setErrorResponse(conn->_response, 413, "Content Too Large", conn->route.cfg);
        return -1;
    }
    return r;
}

void BodyParser::_parseTransferEncoding(Connection* conn) {
    std::string readBufStr = std::string(conn->_readBuf.data(), conn->_readBuf.size());

    if (_transferEncodingState == "readingChunkSize") {
        _lastReadBufStr += readBufStr;

        size_t pos = 0;
        if ((pos = readBufStr.find("\r\n")) == std::string::npos) // test if end of chunk-size
            return;

        if ((_chunkSize = _validateHex(_lastReadBufStr + readBufStr, conn)) == -1)
            return;

        readBufStr = readBufStr.substr(pos + 2);
        _transferEncodingState = "readingChunk";
    }

    conn->_tempBody = readBufStr.substr(0, _chunkSize);

    try {
        readBufStr = readBufStr.substr(_chunkSize + 2);
    } catch (const std::out_of_range& e) {
        return;
    }

    if (readBufStr == "0\r\n\r\n")
        conn->_bodyFinished = true;

    conn->_readBuf.clear();
}

bool BodyParser::_checkType(Connection* conn) {
    BodyContext& bodyCtx = conn->bodyCtx;

    if (conn->_request.headers.find("transfer-encoding") != conn->_request.headers.end())
        bodyCtx.type = BodyContext::Chunked;
    else if (_checkContentLength(conn, bodyCtx))
        bodyCtx.type = BodyContext::ContentLength;
    else
        return false;
    return true;
}

void BodyParser::parse(Connection* conn) {
    while (true) {
        switch (conn->bodyCtx.type) {
        case BodyContext::Undetermined:
            if (!_checkType(conn))
                return;
            break;
        case BodyContext::ContentLength:
            return _parseContentLength(conn);
        case BodyContext::Chunked:
            return _parseTransferEncoding(conn);
        }
    }
}
