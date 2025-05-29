#include "GetHandler.h"
#include "Connection.h"
#include "handlerUtils.h"
#include <sstream>

GetHandler::GetHandler() {}
GetHandler::~GetHandler() {}

bool GetHandler::_getDirectoryListing(const std::string& dirPath, const std::string& requestPath,
                                      std::string& outListing) {
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) {
        return false;
    }

    struct dirent* entry;
    std::ostringstream html;
    html << "<!DOCTYPE html>\n"
         << "<html><head><title>Index of " << requestPath << "</title></head><body>\n"
         << "<h1>Index of " << requestPath << "</h1><ul>\n";
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name == "." || name == "..")
            continue;
        std::string fullPath = dirPath + "/" + name;
        struct stat statbuf;
        if (stat(fullPath.c_str(), &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
            name += "/";
        }
        html << "<li><a href=\"" << requestPath << "/" << name << "\">" << name << "</a></li>\n";
    }
    closedir(dir);
    html << "</ul></body></html>\n";
    outListing = html.str();
    return true;
}

void GetHandler::_serveRegFile(Connection& conn, HttpResponse& resp) {
    setResponse(resp, 200, getMimeType(_path), _pathStat.st_size, new FileBodyProvider(_path.c_str()));
    conn.setState(Connection::SendResponse);
}

void GetHandler::_serveIndexFile(Connection& conn, const RouteConfig& config, HttpResponse& resp) {
    for (size_t i = 0; i < config.index.size(); ++i) {
        std::string indexPath = _path + config.index[i];
        if (stat(indexPath.c_str(), &_pathStat) == 0 && S_ISREG(_pathStat.st_mode)) {
            setResponse(resp, 200, getMimeType(indexPath), _pathStat.st_size, new FileBodyProvider(indexPath.c_str()));
            conn.setState(Connection::SendResponse);
        }
    }
}

void GetHandler::_serveDirectoryListing(Connection& conn, const HttpRequest& request, HttpResponse& resp) {
    std::string listing;
    if (_getDirectoryListing(_path, request.uri, listing)) {
        setResponse(resp, 200, "text/html", listing.size(), new StringBodyProvider(listing));
        conn.setState(Connection::SendResponse);
    }
}

void GetHandler::handle(Connection* conn, const HttpRequest& request, const RouteConfig& config) {
    HttpResponse& resp = conn->_response;
    _path = config.root + request.uri;
    if (!validateRequest(resp, request, config, _path, _pathStat)) {
        conn->setState(Connection::SendResponse);
        return;
    }

    if (S_ISREG(_pathStat.st_mode)) {
        _serveRegFile(*conn, resp);
        return;
    } else if (S_ISDIR(_pathStat.st_mode)) {
        if (!config.index.empty()) {
            _serveIndexFile(*conn, config, resp);
        }
        if (conn->getState() != Connection::SendResponse && config.autoindex) {
            _serveDirectoryListing(*conn, request, resp);
        }
    }
    if (conn->getState() != Connection::SendResponse) {
        setErrorResponse(resp, 403, config);
        conn->setState(Connection::SendResponse);
    }
}
