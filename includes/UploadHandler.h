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
    size_t contentLength;
    std::ofstream* file;
    UploadHandler() : bytesUploaded(0), file(NULL) {}
    ~UploadHandler() { delete file; }

    void uploadNewContent(Connection* conn, size_t contentLength) {
        if ((bytesUploaded + conn->_readBufUsedSize) < contentLength) {
            file->write(reinterpret_cast<const char*>(conn->getReadBuf().data()), conn->_readBufUsedSize);
            bytesUploaded += conn->_readBufUsedSize;
        } else {
            file->write(reinterpret_cast<const char*>(conn->getReadBuf().data()), contentLength - bytesUploaded);
            bytesUploaded = contentLength;
        }
        // need to flush, otherwise it would stay in memory until buffer is full or UploadHandler is
        // deleted
        file->flush();
    }

    virtual void handle(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
        if (!file) {
            file = new std::ofstream(cfg.root + req.uri, std::ios::binary | std::ios::app);
            if (!file->is_open()) {
                std::cerr << "Failed to open file" << std::endl;
                return;
            }
            std::stringstream ss(conn->_request.headers["content-length"]);
            ss >> contentLength;
        }

        if (bytesUploaded < contentLength)
            uploadNewContent(conn, contentLength);
    }
};

#endif // UPLOADHANDLER_H
