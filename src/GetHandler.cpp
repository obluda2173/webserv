#include "GetHandler.h"

bool GetHandler::_getValidation(Connection* conn, HttpRequest& request, RouteConfig& config) {
    if (request.headers.find("content-length") != request.headers.end() ||               // no body in GET method
        request.headers.find("transfer-encoding") != request.headers.end()) {
        return false;
    } else if (stat(_path.c_str(), &_pathStat) != 0) {
        return false;
    }
    // check whether GET is applicable to the provided allow_methods (disscuss with Mr Kay and Mr Yao)
}

void GetHandler::handleGoodResponce(Connection* conn, std::string pathToPut) {      // this should be a private function
    HttpResponse& resp = conn->_response;
    resp.contentLength = _pathStat.st_size;
    resp.body = new FileBodyProvider(pathToPut.c_str());
    resp.statusCode = 200;
    resp.statusMessage = "OK";
    resp.contentType = "text/html";
    resp.contentLanguage = "en-US";
    resp.version = "HTTP/1.1";
    conn->setState(Connection::SendResponse);
}

void GetHandler::handleBadResponce(Connection* conn, int statusCode, std::string statusMessage) {       // this should be a private function
    HttpResponse& resp = conn->_response;
    resp.statusCode = statusCode;
    resp.statusMessage = statusMessage;
    resp.contentType = "text/html";
    resp.contentLanguage = "en-US";
    resp.version = "HTTP/1.1";
    // build a tiny HTML page that includes the code & message
    std::string tempBody =
        "<!DOCTYPE html>\r\n"
        "<html lang=\"en\">\r\n"
        "<head>\r\n"
        "  <meta charset=\"UTF-8\">\r\n"
        "  <title>" + std::to_string(statusCode) + " " + statusMessage + "</title>\r\n"     // we need our own to_string here
        "</head>\r\n"
        "<body>\r\n"
        "  <h1>" + std::to_string(statusCode) + " " + statusMessage + "</h1>\r\n"           // we need our own to_string here
        "  <p>" + statusMessage + ".</p>\r\n"
        "</body>\r\n"
        "</html>\r\n";
    resp.body = new StringBodyProvider(tempBody);
    resp.contentLength = resp.body.size();
    conn->setState(Connection::SendResponse);
}

#include <dirent.h>

std::string generateDirectoryIndexHtml(const std::string& dirPath, const std::string& requestPath) {
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) {         // we don't need this if at all
        return "<html><body><h1>403 Forbidden</h1></body></html>";
    }

    std::vector<std::string> entries;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;

        // Skip . and ..
        if (name == "." || name == "..")
            continue;

        std::string fullPath = dirPath + "/" + name;
        struct stat statbuf;
        if (stat(fullPath.c_str(), &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
            name += "/";  // Append slash to folders
        }

        entries.push_back(name);
    }
    closedir(dir);

    // Now build HTML
    std::ostringstream html;
    html << "<!DOCTYPE html>\n"
         << "<html><head><title>Index of " << requestPath << "</title></head><body>\n"
         << "<h1>Index of " << requestPath << "</h1><ul>\n";

    for (size_t i = 0; i < entries.size(); ++i) {
        html << "<li><a href=\"" << entries[i] << "\">" << entries[i] << "</a></li>\n";
    }

    html << "</ul></body></html>\n";
    return html.str();
}

void GetHandler::handleAutoIndex(Connection* conn, std::string pathToPut) {      // this should be a private function
    HttpResponse& resp = conn->_response;
    std::string tempBody = generateDirectoryIndexHtml(pathToPut, "");
    resp.contentLength = tempBody.size();
    resp.body = new StringBodyProvider(tempBody);
    resp.statusCode = 200;
    resp.statusMessage = "OK";
    resp.contentType = "text/html";
    resp.contentLanguage = "en-US";
    resp.version = "HTTP/1.1";
    conn->setState(Connection::SendResponse);
}

void GetHandler::handle(Connection* conn, HttpRequest& request, RouteConfig& config) {
    HttpResponse& resp = conn->_response;
    _path = config.root + request.uri;

    if (!_getValidation(conn, request, config)) {
        // error message send
    }
    if (S_ISREG(_pathStat.st_mode)) {                       // file
        resp.contentLength = _pathStat.st_size;
        resp.body = new FileBodyProvider(_path.c_str());
        resp.statusCode = 200;
        resp.statusMessage = "OK";
        resp.contentType = "text/html";
        resp.contentLanguage = "en-US";
        resp.version = "HTTP/1.1";
        conn->setState(Connection::SendResponse);
        return;
    } else if (S_ISDIR(_pathStat.st_mode)) {               // directory
        if (!config.index.empty()) {
            for (std::vector<std::string>::const_iterator it = config.index.begin(); it != config.index.end(); ++it) {
                std::string indexPath = _path + *it;
                if (stat((indexPath).c_str(), &_pathStat) == 0) {
                    resp.contentLength = _pathStat.st_size;
                    resp.body = new FileBodyProvider(indexPath.c_str());
                    resp.statusCode = 200;
                    resp.statusMessage = "OK";
                    resp.contentType = "text/html";
                    resp.contentLanguage = "en-US";
                    resp.version = "HTTP/1.1";
                    conn->setState(Connection::SendResponse);
                    return;
                }
            }
        } 
        if (config.autoindex) {
            // if autoindex is on, generate directory listing
        } else {
            // error message with 403 Forbidden 
        }
    }
}
