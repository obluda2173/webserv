#ifndef UPLOADHANDLER_H
#define UPLOADHANDLER_H

#include "HttpRequest.h"
#include "IHandler.h"
#include "RouteConfig.h"
#include <fstream>
#include <sstream>

class UploadHandler : public IHandler {

  public:
    size_t bytesUploaded;
    UploadHandler() : bytesUploaded(0) {}

    virtual void handle(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
        std::ofstream file = std::ofstream(cfg.root + req.uri, std::ios::binary | std::ios::app);
        std::stringstream ss(conn->_request.headers["content-length"]);
        size_t contentLength;
        ss >> contentLength;

        if (bytesUploaded < contentLength) {
            if ((bytesUploaded + conn->_readBufUsedSize) < contentLength) {
                file.write(reinterpret_cast<const char*>(conn->getReadBuf().data()), conn->_readBufUsedSize);
                bytesUploaded += conn->_readBufUsedSize;
            } else {
                file.write(reinterpret_cast<const char*>(conn->getReadBuf().data()), contentLength - bytesUploaded);
                bytesUploaded = contentLength;
            }
        }
    }
};

#endif // UPLOADHANDLER_H
