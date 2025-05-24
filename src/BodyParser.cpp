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

BodyParser::BodyParser() {}

size_t custom_stol(const std::string& str, size_t* idx = 0, int base = 10) {
    char* end;

    const char* start = str.c_str();
    size_t result = strtoll(start, &end, base);

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
    if (conn->_request.headers.find("content-length") == conn->_request.headers.end())
        return false;

    std::stringstream ss(conn->_request.headers["content-length"]);
    ss >> bodyCtx.contentLength;
    if (bodyCtx.contentLength == 0)
        return false;

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
        conn->_readBuf.clear();
    } else {
        int countRest = bodyCtx.contentLength - bodyCtx.bytesReceived;
        conn->_tempBody = std::string(readBuf.data(), bodyCtx.contentLength - bodyCtx.bytesReceived);
        bodyCtx.bytesReceived = bodyCtx.contentLength;
        conn->_bodyFinished = true;
        conn->bodyCtx.type = BodyContext::Undetermined;
        readBuf.advance(countRest);
    }
}

bool BodyParser::_validateHex(size_t& chunkSize, std::string readBufStr, Connection* conn) {
    try {
        chunkSize = custom_stol(readBufStr, NULL, 16);
    } catch (const std::invalid_argument& e) {
        conn->_bodyFinished = true;
        conn->setState(Connection::SendResponse);
        setErrorResponse(conn->_response, 400, "Bad Request", conn->route.cfg);
        conn->bodyCtx.type = BodyContext::Undetermined;
        return false;
    } catch (const std::out_of_range& e) {
        conn->_bodyFinished = true;
        conn->setState(Connection::SendResponse);
        setErrorResponse(conn->_response, 413, "Content Too Large", conn->route.cfg);
        conn->bodyCtx.type = BodyContext::Undetermined;
        return false;
    }
    return true;
}

void BodyParser::_parseChunk(Connection* conn) {
    BodyContext& ctx = conn->bodyCtx;
    std::string newTmp = ctx._bodyBuf.substr(0, ctx._chunkRestSize);
    conn->_tempBody += newTmp;
    ctx._chunkRestSize -= newTmp.size();

    ctx._bodyBuf = ctx._bodyBuf.substr(newTmp.size());
    if (ctx._chunkRestSize != 0)
        return;

    ctx._transferEncodingState = BodyContext::VerifyCarriageReturn;
}

void BodyParser::_verifyCarriageReturn(Connection* conn) {
    BodyContext& ctx = conn->bodyCtx;
    if (ctx._bodyBuf.length() < 2)
        return;

    if (ctx._bodyBuf.substr(0, 2) != "\r\n") {
        conn->_bodyFinished = true;
        conn->setState(Connection::SendResponse);
        setErrorResponse(conn->_response, 400, "Bad Request", conn->route.cfg);
        conn->bodyCtx.type = BodyContext::Undetermined;
        return;
    }

    if (ctx._currentChunkSize == 0) {
        conn->_bodyFinished = true;
        conn->_readBuf.assign(ctx._bodyBuf.substr(2));
        conn->bodyCtx.type = BodyContext::Undetermined;
        return;
    }

    ctx._bodyBuf = ctx._bodyBuf.substr(2);
    ctx._transferEncodingState = BodyContext::ReadingChunkSize;
}

void BodyParser::_parseChunkSize(Connection* conn) {
    BodyContext& ctx = conn->bodyCtx;
    size_t pos = 0;
    if ((pos = ctx._bodyBuf.find("\r\n")) == std::string::npos) // test if end of chunk-size
        return;

    if (!_validateHex(ctx._currentChunkSize, ctx._bodyBuf, conn))
        return;
    ctx._chunkRestSize = ctx._currentChunkSize;

    ctx._bodyBuf = ctx._bodyBuf.substr(pos + 2);
    ctx._transferEncodingState = BodyContext::ReadingChunk;
}

void BodyParser::_parseTransferEncoding(Connection* conn) {
    conn->bodyCtx._bodyBuf += std::string(conn->_readBuf.data(), conn->_readBuf.size());
    conn->_readBuf.clear();

    BodyContext& ctx = conn->bodyCtx;
    bool continueProcessing = true;
    BodyContext::TE_STAGE current_stage;
    while (continueProcessing) {
        current_stage = ctx._transferEncodingState;
        switch (ctx._transferEncodingState) {
        case BodyContext::ReadingChunkSize:
            _parseChunkSize(conn);
            continueProcessing = (ctx._transferEncodingState != current_stage);
            break;
        case BodyContext::ReadingChunk:
            _parseChunk(conn);
            continueProcessing = (ctx._transferEncodingState != current_stage);
            break;
        case BodyContext::VerifyCarriageReturn:
            _verifyCarriageReturn(conn);
            continueProcessing = (ctx._transferEncodingState != current_stage);
            break;
        }
    }

    return;
}

bool BodyParser::_checkType(Connection* conn) {
    BodyContext& bodyCtx = conn->bodyCtx;

    if (conn->_request.headers.find("transfer-encoding") != conn->_request.headers.end()) {
        bodyCtx.type = BodyContext::TransferEncoding;
        return true;
    }
    if (_checkContentLength(conn, bodyCtx)) {
        bodyCtx.type = BodyContext::ContentLength;
        return true;
    }
    conn->_bodyFinished = true;
    return false;
}

void BodyParser::parse(Connection* conn) {
    conn->_tempBody = "";
    while (true) {
        switch (conn->bodyCtx.type) {
        case BodyContext::Undetermined:
            if (!_checkType(conn))
                return;
            break;
        case BodyContext::ContentLength:
            return _parseContentLength(conn);
        case BodyContext::TransferEncoding:
            return _parseTransferEncoding(conn);
        }
    }
}
