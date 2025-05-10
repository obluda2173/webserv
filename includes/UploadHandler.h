#ifndef UPLOADHANDLER_H
#define UPLOADHANDLER_H

#include "HttpRequest.h"
#include "IHandler.h"
#include "RouteConfig.h"
#include <fstream>

class UploadHandler : public IHandler {
  public:
    virtual void handle(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
        conn->getReadBuf();

        std::ofstream file = std::ofstream("tests/unittests/test_files/UploadHandler/example.txt", std::ios::binary);
        file.write(reinterpret_cast<const char*>(conn->getReadBuf().data()), conn->_readBufUsedSize);
        (void)req;
        (void)cfg;
    }
};

#endif // UPLOADHANDLER_H
