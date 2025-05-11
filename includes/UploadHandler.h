#ifndef UPLOADHANDLER_H
#define UPLOADHANDLER_H

#include "Connection.h"
#include "HttpRequest.h"
#include "IHandler.h"
#include "RouteConfig.h"
#include <fstream>
#include <sstream>

class UploadHandler : public IHandler {

  public:
    void uploadNewContent(Connection* conn // , size_t contentLength
    ) {
        ConnectionContext& ctx = conn->ctx;
        if ((ctx.bytesUploaded + conn->_readBufUsedSize) < ctx.contentLength) {
            ctx.file->write(reinterpret_cast<const char*>(conn->getReadBuf().data()), conn->_readBufUsedSize);
            ctx.bytesUploaded += conn->_readBufUsedSize;
        } else {
            ctx.file->write(reinterpret_cast<const char*>(conn->getReadBuf().data()),
                            ctx.contentLength - ctx.bytesUploaded);
            ctx.bytesUploaded = ctx.contentLength;
        }
    }

    virtual void handle(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
        ConnectionContext& ctx = conn->ctx;
        if (!ctx.file) {
            ctx.file = new std::ofstream(cfg.root + req.uri, std::ios::binary | std::ios::app);
            if (!ctx.file->is_open()) {
                std::cerr << "Failed to open file" << std::endl;
                return;
            }
            std::stringstream ss(conn->_request.headers["content-length"]);
            ss >> ctx.contentLength;
        }

        if (ctx.bytesUploaded < ctx.contentLength)
            uploadNewContent(conn);
    }
};

#endif // UPLOADHANDLER_H
