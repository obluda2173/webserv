#include "UploadHandler.h"
#include "HttpRequest.h"
#include "RouteConfig.h"
#include "handlerUtils.h"
#include <fstream>
#include <iostream>
#include <sstream>

UploadHandler::~UploadHandler() {}

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
    std::string path = (cfg.root + req.uri);
    std::string dirPath;

    if (stat(path.c_str(), &statStruct) == 0 && S_ISREG(statStruct.st_mode)) {
        conn->uploadCtx.fileExisted = 1;        // everything is fine
    }
    if (stat(path.c_str(), &statStruct) != 0) {
        if (path.find_last_of('/') == path.size() - 1) {
            path.erase(path.end() - 1);
            dirPath = path.substr(0, path.find_last_of('/'));
            dirPath = dirPath.substr(0, path.find_last_of('/'));
        } else {
            dirPath = path.substr(0, path.find_last_of('/'));
        }
        if (stat(dirPath.c_str(), &statStruct) == 0 && S_ISDIR(statStruct.st_mode)) {
            conn->uploadCtx.fileExisted = 0;        // we need to creat a file
        } else {
            conn->uploadCtx.fileExisted = 2;        // wrong path, we need to return error
        }
    }
    if (path.find("..") != std::string::npos) {     // we do not allow /../appear
        conn->uploadCtx.fileExisted = 3;
        // return;
    }


    // remove the file first and then add the new file
    std::remove((cfg.root + req.uri).c_str());

    UploadContext& ctx = conn->uploadCtx;
    ctx.file = new std::ofstream((cfg.root + req.uri).c_str(), std::ios::binary | std::ios::app);
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

    if (conn->uploadCtx.fileExisted == 1)
        setResponse(conn->_response, 200, "OK", "", 0, NULL);
    if (conn->uploadCtx.fileExisted == 0)
        setResponse(conn->_response, 201, "Created", "", 0, NULL);
    if (conn->uploadCtx.fileExisted == 2 || conn->uploadCtx.fileExisted == 3)
        setErrorResponse(conn->_response, 400, "Bad Request", cfg);

    uploadNewContent(conn);
}
