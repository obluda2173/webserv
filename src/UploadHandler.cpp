#include "UploadHandler.h"
#include "HttpRequest.h"
#include "RouteConfig.h"
#include "handlerUtils.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>

UploadHandler::~UploadHandler() {}

void UploadHandler::uploadNewContent(Connection* conn) {
    UploadContext& ctx = conn->uploadCtx;
    if ((ctx.bytesUploaded + conn->_readBuf.size()) < ctx.contentLength) {
        ctx.file->write(reinterpret_cast< const char* >(conn->getReadBuf().data()), conn->_readBuf.size());
        ctx.bytesUploaded += conn->_readBuf.size();
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

bool UploadHandler::_validateDir(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
    struct stat statStruct;
    std::string path = (cfg.root + req.uri);
    std::string dirPath;
    dirPath = path.substr(0, path.find_last_of('/'));
    bool exists = !stat(dirPath.c_str(), &statStruct);
    if (exists) {
        if (S_ISDIR(statStruct.st_mode)) {
            conn->uploadCtx.fileExisted = false; // everything is fine
            return true;
        }
        setErrorResponse(conn->_response, 409, "Conflict", cfg);
        return false;
    }
    setErrorResponse(conn->_response, 404, "Not Found", cfg);
    return false;
}

bool UploadHandler::_validateFile(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
    struct stat statStruct;
    std::string path = (cfg.root + req.uri);
    std::string dirPath;

    bool exists = !stat(path.c_str(), &statStruct);
    if (exists) {
        if (S_ISREG(statStruct.st_mode)) {
            conn->uploadCtx.fileExisted = true; // everything is fine
            return true;
        }
        setErrorResponse(conn->_response, 409, "Conflict", cfg);
        return false;
    }

    if (_validateDir(conn, req, cfg))
        return true;
    return false;
}

bool UploadHandler::_validation(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
    if (!_validateContentLength(conn, cfg))
        return false;

    if (!_validateFile(conn, req, cfg))
        return false;

    return true;
}

void UploadHandler::_initUploadCxt(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
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
    if (conn->uploadCtx.state.empty()) {
        if (!_validation(conn, req, cfg))
            return;
    }
    conn->uploadCtx.state = "After Validation";

    if (!conn->uploadCtx.file) {
        // remove the file first and then add the new file
        std::remove((cfg.root + req.uri).c_str());
        _initUploadCxt(conn, req, cfg);
    }

    uploadNewContent(conn);

    if (conn->uploadCtx.fileExisted)
        setResponse(conn->_response, 200, "OK", "", 0, NULL);
    else
        setResponse(conn->_response, 201, "Created", "", 0, NULL);
}
