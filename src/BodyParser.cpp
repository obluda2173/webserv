#include "BodyParser.h"

void BodyParser::parse(Connection* conn) {
    conn->_bodyFinished = true;
    conn->_tempBody = std::string(conn->_readBuf.data(), conn->_readBuf.size());
}
