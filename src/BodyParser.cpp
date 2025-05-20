#include "BodyParser.h"
#include "Connection.h"
#include <sstream>

void BodyParser::parse(Connection* conn) {
    BodyContext& bodyCtx = conn->bodyCtx;
    Buffer& readBuf = conn->_readBuf;

    if (bodyCtx.contentLength == 0) {
        std::stringstream ss(conn->_request.headers["content-length"]);
        ss >> bodyCtx.contentLength;
    }

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
