#include "handlerUtils.h"
#include "utils.h"
#include <sstream>

std::map< std::string, std::string > mimeTypes;
struct MimeInitializer {
    MimeInitializer() {
        mimeTypes[".html"] = "text/html";
        mimeTypes[".htm"] = "text/html";
        mimeTypes[".txt"] = "text/plain";
        mimeTypes[".css"] = "text/css";
        mimeTypes[".js"] = "application/javascript";
        mimeTypes[".jpg"] = "image/jpeg";
        mimeTypes[".jpeg"] = "image/jpeg";
        mimeTypes[".png"] = "image/png";
        mimeTypes[".gif"] = "image/gif";
        mimeTypes[".pdf"] = "application/pdf";
    }
};
static MimeInitializer mimeInit;

std::map< int, std::string > statusPhrases;
struct PhraseInitializer {
    PhraseInitializer() {
        // 2xx Success
        statusPhrases[200] = "OK";
        statusPhrases[201] = "Created";
        statusPhrases[202] = "Accepted";
        statusPhrases[204] = "No Content";

        // 3xx Redirection
        statusPhrases[301] = "Moved Permanently";
        statusPhrases[302] = "Found";
        statusPhrases[303] = "See Other";
        statusPhrases[304] = "Not Modified";
        statusPhrases[307] = "Temporary Redirect";
        statusPhrases[308] = "Permanent Redirect";

        // 4xx Client Error
        statusPhrases[400] = "Bad Request";
        statusPhrases[401] = "Unauthorized";
        statusPhrases[403] = "Forbidden";
        statusPhrases[404] = "Not Found";
        statusPhrases[405] = "Method Not Allowed";
        statusPhrases[408] = "Request Timeout";
        statusPhrases[409] = "Conflict";
        statusPhrases[413] = "Payload Too Large";
        statusPhrases[414] = "URI Too Long";
        statusPhrases[429] = "Too Many Requests";

        // 5xx Server Error
        statusPhrases[500] = "Internal Server Error";
        statusPhrases[501] = "Not Implemented";
        statusPhrases[502] = "Bad Gateway";
        statusPhrases[503] = "Service Unavailable";
        statusPhrases[504] = "Gateway Timeout";
    }
};
static PhraseInitializer phrasesInit;

std::string getMimeType(const std::string& path) {
    std::string ext = "";
    std::string::size_type pos = path.rfind('.');
    if (pos != std::string::npos) {
        ext = path.substr(pos);
    }
    std::map< std::string, std::string >::const_iterator it = mimeTypes.find(ext);
    if (it != mimeTypes.end()) {
        return it->second;
    }
    return DEFAULT_MIME_TYPE;
}

int hexToInt(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return 10 + c - 'a';
    return -1;
}

std::string decodePercent(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%' && i + 2 < str.size()) {
            int hi = hexToInt(tolower(str[i + 1]));
            int lo = hexToInt(tolower(str[i + 2]));
            if (hi != -1 && lo != -1) {
                result += static_cast< char >((hi << 4) | lo);
                i += 2;
                continue;
            }
        }
        result += str[i];
    }
    return result;
}

std::string normalizePath(const std::string& root, const std::string& uri) {
    std::istringstream iss(uri.substr(0, uri.find('?')));
    std::vector< std::string > segments;
    std::string encoded_segment;

    while (std::getline(iss, encoded_segment, '/')) {
        std::string segment = decodePercent(encoded_segment);
        if (segment.empty() || segment == ".")
            continue;
        if (segment == "..") {
            if (!segments.empty())
                segments.pop_back();
        } else {
            segments.push_back(segment);
        }
    }

    std::ostringstream oss;
    oss << root;
    for (size_t i = 0; i < segments.size(); ++i) {
        oss << '/' << segments[i];
    }
    std::string result = oss.str();

    if (uri == "/" && segments.empty()) {
        result += "/";
    }
    return (result.find(root) == 0) ? result : "";
}

void setHeader(HttpResponse& resp, std::string key, std::string value) { resp.headers[toLower(key)] = value; }

void setResponse(HttpResponse& resp, int statusCode, const std::string& contentType, size_t contentLength,
                 IBodyProvider* bodyProvider) {
    resp.version = DEFAULT_HTTP_VERSION;
    resp.statusCode = statusCode;
    resp.statusMessage = statusPhrases.at(statusCode);
    resp.contentType = contentType;
    resp.contentLanguage = DEFAULT_CONTENT_LANGUAGE;
    resp.contentLength = contentLength;
    resp.body = bodyProvider;
}

void setErrorResponse(HttpResponse& resp, int code, const RouteConfig& config) {
    std::map< int, std::string >::const_iterator it = config.errorPage.find(code);
    if (it != config.errorPage.end()) {
        std::string errorPagePath = config.root + it->second;
        struct stat fileStat;
        if (stat(errorPagePath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
            setResponse(resp, code, getMimeType(errorPagePath), fileStat.st_size,
                        new FileBodyProvider(errorPagePath.c_str()));
            return;
        }
    }
    setResponse(resp, code, mimeTypes[".txt"], 0, NULL);
}

bool validateRequest(HttpResponse& resp, const HttpRequest& req, const RouteConfig& config, std::string& path,
                     struct stat& pathStat) {

    if (req.method != "GET" && req.method != "POST" && req.method != "DELETE") {
        setErrorResponse(resp, 501, config);
        return false;
    }

    // 2. URI Validation
    if (req.uri.empty() || req.uri.length() > MAX_URI_LENGTH) {
        setErrorResponse(resp, 400, config);
        return false;
    }

    // 3. Path Normalization & Security
    if (path.empty() || path.find(config.root) != 0) {
        setErrorResponse(resp, 403, config);
        return false;
    }

    // 4. Body Content Validation
    if ((req.headers.count("content-length") || req.headers.count("transfer-encoding")) &&
        (req.method == "GET" || req.method == "DELETE")) {
        setErrorResponse(resp, 400, config);
        return false;
    }

    // 5. Resource Existence Check
    if (stat(path.c_str(), &pathStat) != 0) {
        std::cout << path << std::endl;
        setErrorResponse(resp, 404, config);
        return false;
    }

    // 6. Resource Type Validation
    if ((req.method == "DELETE" && !S_ISREG(pathStat.st_mode) && !S_ISDIR(pathStat.st_mode)) ||
        (req.method == "GET" && !S_ISREG(pathStat.st_mode) && !S_ISDIR(pathStat.st_mode)) ||
        (req.method == "POST" && S_ISDIR(pathStat.st_mode))) {
        setErrorResponse(resp, 400, config);
        return false;
    }

    // 7. Access Permissions
    int accessMode = F_OK;
    if (req.method == "GET")
        accessMode |= R_OK;
    else if (req.method == "POST")
        accessMode |= W_OK;
    else if (req.method == "DELETE")
        accessMode |= W_OK;
    if (access(path.c_str(), accessMode) != 0) {
        setErrorResponse(resp, 403, config);
        return false;
    }

    return true;
}
