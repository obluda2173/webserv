#include "BodyParser.h"
#include "Connection.h"
#include <sstream>

void BodyParser::parse(Connection* conn) {
    BodyContext& bodyCtx = conn->bodyCtx;

    if (bodyCtx.contentLength == 0) {
        std::stringstream ss(conn->_request.headers["content-length"]);
        ss >> bodyCtx.contentLength;
    }

    conn->_tempBody = std::string(conn->_readBuf.data(), conn->_readBuf.size());
    bodyCtx.bytesReceived += conn->_tempBody.size();
    if (bodyCtx.bytesReceived == bodyCtx.contentLength) {
        conn->_bodyFinished = true;
    }
}
