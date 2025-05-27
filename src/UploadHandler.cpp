#include "UploadHandler.h"
#include "Connection.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "RouteConfig.h"
#include "handlerUtils.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>

UploadHandler::~UploadHandler() {}

void UploadHandler::uploadNewContent(Connection* conn) {
    UploadContext& ctx = conn->uploadCtx;
    ctx.file->write(conn->_tempBody.data(), conn->_tempBody.size());
    if (conn->_bodyFinished)
        ctx.state = UploadContext::UploadFinished;
}

bool UploadHandler::_validateContentLength(Connection* conn, const RouteConfig& cfg) {
    size_t contentLength;
    if (conn->_request.headers.find("content-length") == conn->_request.headers.end()) {
        setErrorResponse(conn->_response, 400, cfg);
        return false;
    }

    std::stringstream ss(conn->_request.headers["content-length"]);
    ss >> contentLength;

    if (contentLength == 0) {
        setErrorResponse(conn->_response, 400, cfg);
        return false;
    }

    if (contentLength > cfg.clientMaxBody) {
        setErrorResponse(conn->_response, 413, cfg);
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
        if (!S_ISDIR(statStruct.st_mode)) {
            setErrorResponse(conn->_response, 409, cfg);
            return false;
        }
        conn->uploadCtx.fileExisted = false; // everything is fine
        if (access(dirPath.c_str(), W_OK) == -1) {
            setErrorResponse(conn->_response, 403, cfg);
            return false;
        }
        return true;
    }

    setErrorResponse(conn->_response, 404, cfg);
    return false;
}

bool UploadHandler::_validateFile(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
    struct stat statStruct;
    std::string dirPath;
    std::string path = (cfg.root + req.uri);
    bool exists = !stat(path.c_str(), &statStruct);
    if (exists) {
        if (!S_ISREG(statStruct.st_mode)) {
            setErrorResponse(conn->_response, 409, cfg); // i.e. if the file is a directory
            return false;
        }

        if (access(path.c_str(), W_OK) == -1) {
            setErrorResponse(conn->_response, 403, cfg);
            return false;
        }

        conn->uploadCtx.fileExisted = true; // everything is fine
        return true;
    }

    if (_validateDir(conn, req, cfg))
        return true;
    return false;
}

bool UploadHandler::_validateNotActive(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
    std::string path = (cfg.root + req.uri);
    if (_activeUploadPaths.find(path) != _activeUploadPaths.end()) {
        setErrorResponse(conn->_response, 409, cfg);
        return false;
    }
    _activeUploadPaths.insert(path);
    return true;
}

bool UploadHandler::_validation(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
    if (!_validateNotActive(conn, req, cfg))
        return false;

    if (!_validateFile(conn, req, cfg))
        return false;

    return true;
}

bool UploadHandler::_initUploadCxt(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
    UploadContext& ctx = conn->uploadCtx;
    ctx.file = new std::ofstream((cfg.root + req.uri + ".temp").c_str(), std::ios::binary | std::ios::app);
    if (!ctx.file->is_open()) {
        std::cerr << "Failed to open file" << std::endl;
        setErrorResponse(conn->_response, 500, cfg);
        return false;
    }
    return true;
}

void UploadHandler::_renameOrRemoveFile(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
    conn->uploadCtx.file->close();
    if (conn->uploadCtx.fileExisted) {
        std::remove((cfg.root + req.uri).c_str());
        rename((cfg.root + req.uri + ".temp").c_str(), (cfg.root + req.uri).c_str());
        setResponse(conn->_response, 200, "", 0, NULL);
    } else {
        rename((cfg.root + req.uri + ".temp").c_str(), (cfg.root + req.uri).c_str());
        std::string content = "Created " + req.uri;
        setResponse(conn->_response, 201, "text/plain", content.length(), new StringBodyProvider(content));
    }
}

void UploadHandler::handle(Connection* conn, const HttpRequest& req, const RouteConfig& cfg) {
    while (true) {
        UploadContext& uploadCtx = conn->uploadCtx;
        switch (uploadCtx.state) {
        case UploadContext::Validation:
            if (!_validation(conn, req, cfg))
                return conn->setState(Connection::SendResponse);
            uploadCtx.state = UploadContext::Initialising;
            break; // will fallthrough
        case UploadContext::Initialising:
            if (!_initUploadCxt(conn, req, cfg))
                return conn->setState(Connection::SendResponse);
            uploadCtx.state = UploadContext::Uploading;
            break; // will fallthrough
        case UploadContext::Uploading:
            uploadCtx.file->write(conn->_tempBody.data(), conn->_tempBody.size());
            if (!conn->_bodyFinished)
                return;
            uploadCtx.state = UploadContext::UploadFinished;
            break; // will fallthrough
        case UploadContext::UploadFinished:
            _activeUploadPaths.erase(cfg.root + req.uri);
            _renameOrRemoveFile(conn, req, cfg);
            delete uploadCtx.file;
            uploadCtx.file = NULL;
            uploadCtx.state = UploadContext::Validation;
            conn->setState(Connection::SendResponse);
            return;
        }
    }
}
