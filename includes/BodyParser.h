#ifndef BODYPARSER_H
#define BODYPARSER_H

#include "Connection.h"

class BodyParser {
  private:
    std::string _transferEncodingState;
    // long long _chunkBytesRead;
    size_t _chunkSize;
    std::string _lastChunkSizeStr;
    bool _checkContentLength(Connection* conn, BodyContext& bodyCtx);
    void _parseContentLength(Connection* conn);
    bool _checkType(Connection* conn);
    bool _validateHex(size_t& chunkSize, std::string readBufStr, Connection* conn);
    void _parseTransferEncoding(Connection* conn);
    void _parseChunkSize(std::string& readBufStr, Connection* conn);
    void _parseChunk(std::string& readBufStr, Connection* conn);
    void _verifyCarriageReturn(std::string& readBufStr, Connection* conn);

  public:
    BodyParser();
    void parse(Connection* conn);
};

#endif // BODYPARSER_H
