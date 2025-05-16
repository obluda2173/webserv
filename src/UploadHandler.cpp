#include "UploadHandler.h"
#include "Connection.h"
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
        ctx.file->write(reinterpret_cast< const char* >(conn->_readBuf.data()), conn->_readBuf.size());
        ctx.bytesUploaded += conn->_readBuf.size();
        conn->_readBuf.clear();
    } else {
        ctx.file->write(reinterpret_cast< const char* >(conn->_readBuf.data()), ctx.contentLength - ctx.bytesUploaded);
        ctx.bytesUploaded = ctx.contentLength;
        conn->_readBuf.clear();
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
            // if (access(dirPath.c_str(), W_OK) == -1) {
            //     setErrorResponse(conn->_response, 403, "Forbidden", cfg);
            //     return false;
            // }
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
    std::string dirPath;
    std::string path = (cfg.root + req.uri);
    bool exists = !stat(path.c_str(), &statStruct);
    if (exists) {
        if (S_ISREG(statStruct.st_mode)) {
            conn->uploadCtx.fileExisted = true; // everything is fine
            if (access(path.c_str(), W_OK) == -1) {
                setErrorResponse(conn->_response, 403, "Forbidden", cfg);
                return false;
            }
            return true;
        }
        setErrorResponse(conn->_response, 409, "Conflict", cfg);
        return false;
    }

    if (_validateDir(conn, req, cfg))
        return true;
    return false;
}

bool UploadHandler::_validateNotActive(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
    std::string path = (cfg.root + req.uri);
    if (_activeUploadPaths.find(path) != _activeUploadPaths.end()) {
        setErrorResponse(conn->_response, 409, "Conflict", cfg);
        return false;
    }
    _activeUploadPaths.insert(path);
    return true;
}

bool UploadHandler::_validation(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
    if (!_validateContentLength(conn, cfg))
        return false;

    if (!_validateNotActive(conn, req, cfg))
        return false;

    if (!_validateFile(conn, req, cfg))
        return false;

    return true;
}

bool UploadHandler::_initUploadCxt(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
    UploadContext& ctx = conn->uploadCtx;
    ctx.file = new std::ofstream((cfg.root + req.uri).c_str(), std::ios::binary | std::ios::app);
    if (!ctx.file->is_open()) {
        std::cerr << "Failed to open file" << std::endl;
        setErrorResponse(conn->_response, 500, "Internal Server Error", cfg);
        return false;
    }
    std::stringstream ss(conn->_request.headers["content-length"]);
    ss >> ctx.contentLength;
    return true;
}

void UploadHandler::handle(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
    while (true) {
        UploadContext& uploadCtx = conn->uploadCtx;
        switch (uploadCtx.state) {
        case UploadContext::Validation:
            if (!_validation(conn, req, cfg))
                return;
            uploadCtx.state = UploadContext::Initialising;
            break; // will fallthrough
        case UploadContext::Initialising:
            std::remove((cfg.root + req.uri).c_str());
            if (!_initUploadCxt(conn, req, cfg))
                return;
            uploadCtx.state = UploadContext::Uploading;
            break; // will fallthrough
        case UploadContext::Uploading:
            uploadNewContent(conn);
            if (uploadCtx.bytesUploaded < uploadCtx.contentLength)
                return;
            uploadCtx.state = UploadContext::UploadFinished;
            break; // will fallthrough
        case UploadContext::UploadFinished:
            _activeUploadPaths.erase(cfg.root + req.uri);
            if (conn->uploadCtx.fileExisted)
                setResponse(conn->_response, 200, "OK", "", 0, NULL);
            else
                setResponse(conn->_response, 201, "Created", "", 0, NULL);
            return;
        }
    }
}
