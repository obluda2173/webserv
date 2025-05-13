#include "UploadHandler.h"
#include "HttpRequest.h"
#include "RouteConfig.h"
#include "handlerUtils.h"
#include <fstream>
#include <iostream>
#include <sstream>

void UploadHandler::uploadNewContent(Connection* conn) {
    UploadContext& ctx = conn->uploadCtx;
    if ((ctx.bytesUploaded + conn->_readBufUsedSize) < ctx.contentLength) {
        ctx.file->write(reinterpret_cast< const char* >(conn->getReadBuf().data()), conn->_readBufUsedSize);
        ctx.bytesUploaded += conn->_readBufUsedSize;
    } else {
        ctx.file->write(reinterpret_cast< const char* >(conn->getReadBuf().data()),
                        ctx.contentLength - ctx.bytesUploaded);
        ctx.bytesUploaded = ctx.contentLength;
    }
}

bool UploadHandler::_validateContentLength(Connection* conn, const RouteConfig& cfg) {
    size_t contentLength;
    if (conn->_request.headers.find("content-length") == conn->_request.headers.end()) {
        setErrorResponse(conn->_response, 400, "Bad Request", cfg);
        return false;
    }

    std::stringstream ss(conn->_request.headers["content-length"]);
    ss >> contentLength;

    if (contentLength == 0) {
        setErrorResponse(conn->_response, 400, "Bad Request", cfg);
        return false;
    }

    if (contentLength > cfg.clientMaxBody) {
        setErrorResponse(conn->_response, 413, "Content Too Large", cfg);
        return false;
    }

    return true;
}

bool UploadHandler::_validation(Connection* conn, const RouteConfig& cfg) {
    if (!_validateContentLength(conn, cfg))
        return false;

    return true;
}

void UploadHandler::_initUploadCxt(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
    struct stat statStruct;
    conn->uploadCtx.fileExisted = (stat((cfg.root + req.uri).c_str(), &statStruct) == 0 && S_ISREG(statStruct.st_mode));
    std::remove((cfg.root + req.uri).c_str());

    UploadContext& ctx = conn->uploadCtx;
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

    if (!conn->uploadCtx.file)
        _initUploadCxt(conn, req, cfg);

    if (conn->uploadCtx.fileExisted)
        setResponse(conn->_response, 200, "OK", "", 0, NULL);
    else
        setResponse(conn->_response, 201, "Created", "", 0, NULL);

    uploadNewContent(conn);
}
