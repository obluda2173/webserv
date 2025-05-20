#ifndef BODYPARSER_H
#define BODYPARSER_H

#include "Connection.h"

class BodyParser {
  private:
    bool _checkContentLength(Connection* conn, BodyContext& bodyCtx);

  public:
    void parse(Connection* conn);
};

#endif // BODYPARSER_H
