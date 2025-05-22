#include "BodyParser.h"
#include "Connection.h"
#include "handlerUtils.h"
#include <sstream>
#include <stdexcept>
#include <string>

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
void BodyParser::_parseChunked(Connection* conn) {
    std::string readBufStr = std::string(conn->_readBuf.data(), conn->_readBuf.size());
    long long r;
    try {
        r = std::stol(readBufStr, nullptr, 16);
    } catch (const std::invalid_argument& e) {
        conn->setState(Connection::SendResponse);
        setErrorResponse(conn->_response, 404, "Bad Request", conn->route.cfg);
    }

    size_t pos = readBufStr.find("\r\n");

    conn->_tempBody = readBufStr.substr(pos + 2, r);
    conn->_readBuf.clear();
    conn->_bodyFinished = true;
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
            return _parseChunked(conn);
        }
    }
}
