#include "UploadHandler.h"
#include "HttpRequest.h"
#include "RouteConfig.h"
#include <fstream>
#include <sstream>

void UploadHandler::uploadNewContent(Connection* conn) {
    ConnectionContext& ctx = conn->ctx;
    if ((ctx.bytesUploaded + conn->_readBufUsedSize) < ctx.contentLength) {
        ctx.file->write(reinterpret_cast< const char* >(conn->getReadBuf().data()), conn->_readBufUsedSize);
        ctx.bytesUploaded += conn->_readBufUsedSize;
    } else {
        ctx.file->write(reinterpret_cast< const char* >(conn->getReadBuf().data()),
                        ctx.contentLength - ctx.bytesUploaded);
        ctx.bytesUploaded = ctx.contentLength;
    }
}

bool UploadHandler::_validation(Connection* conn, const RouteConfig& cfg) {
    std::stringstream ss(conn->_request.headers["content-length"]);
    size_t contentLength;
    ss >> contentLength;

    if (contentLength > cfg.clientMaxBody) {
        conn->_response.statusCode = 413;
        return false;
    }
    conn->_response.statusCode = 201;
    return true;
}

void UploadHandler::_initUploadCxt(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
    ConnectionContext& ctx = conn->ctx;
    ctx.file = new std::ofstream(cfg.root + req.uri, std::ios::binary | std::ios::app);
    if (!ctx.file->is_open()) {
        std::cerr << "Failed to open file" << std::endl;
        return;
    }
    std::stringstream ss(conn->_request.headers["content-length"]);
    ss >> ctx.contentLength;
}

void UploadHandler::handle(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {

    if (!_validation(conn, cfg))
        return;

    if (!conn->ctx.file)
        _initUploadCxt(conn, req, cfg);

    uploadNewContent(conn);
}
