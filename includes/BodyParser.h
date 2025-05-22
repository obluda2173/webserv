#ifndef BODYPARSER_H
#define BODYPARSER_H

#include "Connection.h"

class BodyParser {
  private:
    bool _checkContentLength(Connection* conn, BodyContext& bodyCtx);
    void _parseContentLength(Connection* conn);
    bool _checkType(Connection* conn);
    void _parseChunked(Connection* conn);

  public:
    void parse(Connection* conn);
};

#endif // BODYPARSER_H
