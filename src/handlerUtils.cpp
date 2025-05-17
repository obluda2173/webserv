#include "handlerUtils.h"
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

void setResponse(HttpResponse& resp, int statusCode, const std::string& statusMessage, const std::string& contentType,
                 size_t contentLength, IBodyProvider* bodyProvider) {
    resp.version = DEFAULT_HTTP_VERSION;
    resp.statusCode = statusCode;
    resp.statusMessage = statusMessage;
    resp.contentType = contentType;
    resp.contentLanguage = DEFAULT_CONTENT_LANGUAGE;
    resp.contentLength = contentLength;
    resp.body = bodyProvider;
}

void setErrorResponse(HttpResponse& resp, int code, const std::string& message, const RouteConfig& config) {
    std::map< int, std::string >::const_iterator it = config.errorPage.find(code);
    if (it != config.errorPage.end()) {
        std::string errorPagePath = config.root + it->second;
        struct stat fileStat;
        if (stat(errorPagePath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
            setResponse(resp, code, message, getMimeType(errorPagePath), fileStat.st_size,
                        new FileBodyProvider(errorPagePath.c_str()));
            return;
        }
    }
    setResponse(resp, code, message, mimeTypes[".txt"], message.size(), new StringBodyProvider(message));
}

bool validateRequest(HttpResponse& resp, const HttpRequest& req, const RouteConfig& config, std::string& path,
                     struct stat& pathStat) {
    // 2. URI Validation
    if (req.uri.empty() || req.uri.length() > MAX_URI_LENGTH) {
        setErrorResponse(resp, 400, "Bad Request: Invalid URI", config);
        return false;
    }

    // 3. Path Normalization & Security
    if (path.empty() || path.find(config.root) != 0) {
        setErrorResponse(resp, 403, "Forbidden: Invalid path", config);
        return false;
    }

    // 4. Body Content Validation
    if ((req.headers.count("content-length") || req.headers.count("transfer-encoding")) &&
        (req.method == "GET" || req.method == "DELETE")) {
        setErrorResponse(resp, 400, "Bad Request: Invalid body content", config);
        return false;
    }

    // 5. Resource Existence Check
    if (stat(path.c_str(), &pathStat) != 0) {
        setErrorResponse(resp, 404, "Not Found", config);
        return false;
    }

    // 6. Resource Type Validation
    if ((req.method == "DELETE" && !S_ISREG(pathStat.st_mode) && !S_ISDIR(pathStat.st_mode)) ||
        (req.method == "GET" && !S_ISREG(pathStat.st_mode) && !S_ISDIR(pathStat.st_mode)) ||
        (req.method == "POST" && S_ISDIR(pathStat.st_mode))) {
        setErrorResponse(resp, 400, "Bad Request: Invalid resource type", config);
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
        setErrorResponse(resp, 403, "Forbidden: Access denied", config);
        return false;
    }

    return true;
}
