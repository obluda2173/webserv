#ifndef BODYPARSER_H
#define BODYPARSER_H

#include "Connection.h"

class BodyParser {
  private:
    // long long _chunkBytesRead;
    bool _checkContentLength(Connection* conn, BodyContext& bodyCtx);
    void _parseContentLength(Connection* conn);
    bool _checkType(Connection* conn);
    bool _validateHex(size_t& chunkSize, std::string readBufStr, Connection* conn);
    void _parseTransferEncoding(Connection* conn);
    void _parseChunkSize(Connection* conn);
    void _parseChunk(Connection* conn);
    void _verifyCarriageReturn(Connection* conn);

  public:
    BodyParser();
    void parse(Connection* conn);
};

#endif // BODYPARSER_H
