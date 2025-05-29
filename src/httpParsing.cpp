#include "ILogger.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

bool checkValidMethod(const std::string& method) {
    return method == "GET" || method == "POST" || method == "PUT" || method == "DELETE" || method == "HEAD" ||
           method == "OPTIONS" || method == "PATCH";
}

bool checkMethodImplemented(const std::string& method) {
    return method == "GET" || method == "POST" || method == "DELETE";
}

bool isValidHostString(const std::string& host) {
    // Characters that should not be present in a host
    const char* invalidChars = "/?#[]@!$&'()*+,;=";

    // Check if any invalid character is present
    for (size_t i = 0; invalidChars[i] != '\0'; ++i) {
        if (host.find(invalidChars[i]) != std::string::npos) {
            return false;
        }
    }

    // Special check for space and control characters
    for (size_t i = 0; i < host.length(); ++i) {
        if (host[i] <= 32 || host[i] >= 127) {
            return false;
        }
    }

    return true;
}

std::string parseOutProtocolAndHost(std::string uri) {
    if (uri.empty())
        return "";

    if (uri[0] == '/')
        return uri;

    if (uri.substr(0, 7) != "http://")
        return "";

    if (uri.length() == 7)
        return "";

    uri = uri.substr(7);
    size_t pos = 7;
    pos = uri.find("/", pos);
    if (pos != std::string::npos) {
        if (!isValidHostString(uri.substr(0, pos))) {
            return "";
        }
        return uri.substr(pos);
    }

    if (!isValidHostString(uri.substr(0, pos)))
        return "";
    return "/"; // Return everything after the host
}

bool checkValidVersion(const std::string& version) {
    double ver = 0;
    std::string verStr = version.substr(5);
    std::istringstream iss(verStr);
    iss >> ver;
    if (ver < 1 || ver >= 2) { // I CHANGED THIS FROM 0 TO 1, BECAUSE HTTP/0.9 IS NOT SUPPORTED
        return false;
    }
    return true;
}

bool checkValidVersionSyntax(const std::string& version) {
    if (version.empty()) {
        return false;
    }
    if (version.find("HTTP/") != 0) {
        return false;
    }
    std::string verStr = version.substr(5);
    if (verStr.size() != 3) {
        return false;
    }
    if (!isdigit(verStr[0]) || verStr[1] != '.' || !isdigit(verStr[2])) { // TODO: why check 2 times for a "."?
        return false;
    }
    return true;
}

bool isValidTokenChar(char c) { return isalnum(c) || c == '-' || c == '_' || c == '.'; }

bool checkOptionalCharset(const std::string& str) {
    size_t pos = str.find("charset=");
    if (pos == std::string::npos)
        return true;

    std::string charset = str.substr(pos + 8);
    charset.erase(0, charset.find_first_not_of(" "));
    if (charset.empty())
        return false;

    for (size_t i = 0; i < charset.size(); i++) {
        if (!isValidTokenChar(charset[i]))
            return false;
    }
    return true;
}

bool checkOptionalBoundary(const std::string& str) {
    size_t pos = str.find("boundary=");
    if (pos == std::string::npos)
        return true;

    std::string boundary = str.substr(pos + 9);
    boundary.erase(0, boundary.find_first_not_of(" "));
    if (boundary.empty())
        return false;

    for (size_t i = 0; i < boundary.size(); i++) {
        if (!isValidTokenChar(boundary[i]))
            return false;
    }
    return true;
}

bool isValidContentType(const std::string& str) {
    size_t slash = str.find('/');
    if (slash == std::string::npos || slash == 0 || slash == str.size() - 1)
        return false;

    std::string type = str.substr(0, slash);
    if (type != "application" && type != "text" && type != "image" && type != "audio" && type != "video" &&
        type != "multipart" && type != "font" && type != "model" && type != "message" && type != "example")
        return false;

    // Only validate charset/boundary if present
    if (!checkOptionalCharset(str))
        return false;
    if (type == "multipart" && (!checkOptionalBoundary(str) || str.find("boundary=") == std::string::npos ||
                                str.find(';') == std::string::npos))
        return false;
    return true;
}

bool checkQValue(const std::string& str) {
    size_t pos = str.find("q=");

    while (pos != std::string::npos) {
        pos += 2; // Skip past "q="

        // Skip whitespace
        while (pos < str.size() && str[pos] == ' ')
            ++pos;

        // Must start with digit
        if (pos >= str.size() || !isdigit(str[pos]))
            return false;

        std::string qvalue;
        while (pos < str.size() && (isdigit(str[pos]) || str[pos] == '.'))
            qvalue += str[pos++];

        std::istringstream iss(qvalue);
        double q;
        iss >> q;

        if (iss.fail() || !iss.eof())
            return false;

        if (q < 0.0 || q > 1.0)
            return false;

        // Continue searching (in case there are multiple "q=" tokens)
        pos = str.find("q=", pos);
    }
    return true;
}

bool isValidHost(std::string str) {
    size_t colon = str.find(':');
    if (colon != std::string::npos) {
        std::string host = str.substr(0, colon);
        std::string port = str.substr(colon + 1);
        if (host.empty() || port.empty()) {
            return false;
        }
        for (size_t i = 0; i < port.size(); i++) {
            if (!isdigit(port[i])) {
                return false;
            }
        }
    } else {
        if (str.empty()) {
            return false;
        }
    }
    return true;
}

bool isValidAccept(const std::string& str) {
    std::vector< std::string > tokens;
    std::istringstream iss(str);
    std::string token;
    bool available = false;
    while (std::getline(iss, token, ',')) {
        token.erase(0, token.find_first_not_of(" "));
        tokens.push_back(token);
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        std::string token2 = tokens[i];
        size_t slash = token2.find('/');
        if (slash == std::string::npos || slash == 0 ||
            slash == token2.size() - 1) // the char '/' must be in the middle of the string
            return false;
        std::string type = token2.substr(0, slash);
        if (type == "application" || type == "text" || type == "image" || type == "audio" || type == "video" ||
            type == "multipart" || type == "font" || type == "model" || type == "message" || type == "example" ||
            type == "*") {
            available = true;
        }
        // Only validate charset/boundary if present
        if (!checkOptionalCharset(token2)) {
            return false;
        }
        if (!checkOptionalBoundary(token2)) {
            return false;
        }
        if (!checkQValue(token2)) {
            return false;
        }
    }
    if (available == false) {
        return false;
    }
    return true;
}

bool isValidAcceptEncoding(const std::string& str) {
    std::vector< std::string > tokens;
    std::istringstream iss(str);
    std::string token;
    std::string token3;
    bool available = false;
    while (std::getline(iss, token, ',')) {
        token.erase(0, token.find_first_not_of(" "));
        tokens.push_back(token);
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        std::string token2 = tokens[i];
        size_t semicolon = token2.find(';');
        if (semicolon != std::string::npos) {
            token3 = token2.substr(0, semicolon);
        } else {
            token3 = token2;
        }
        if (token3 == "gzip" || token3 == "deflate" || token3 == "br" || token3 == "compress" || token3 == "identity" ||
            token3 == "zstd") {
            available = true;
        }
        if (checkQValue(token2) == false) {
            return false;
        }
    }
    if (available == false) {
        return false;
    }
    return true;
}

bool isValidAcceptLanguage(const std::string& str) {
    std::vector< std::string > tokens;
    std::istringstream iss(str);
    std::string token;
    std::string lang;
    while (std::getline(iss, token, ',')) {
        token.erase(0, token.find_first_not_of(" "));
        tokens.push_back(token);
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        std::string token2 = tokens[i];
        size_t breakpoint;
        if (token2.find(';') != std::string::npos && token2.find('-') != std::string::npos) {
            if (token2.find(';') < token2.find('-')) {
                breakpoint = token2.find(';');
            } else {
                breakpoint = token2.find('-');
            }
        } else if (token2.find(';') != std::string::npos) {
            breakpoint = token2.find(';');
        } else if (token2.find('-') != std::string::npos) {
            breakpoint = token2.find('-');
        } else {
            breakpoint = token2.length();
        }
        lang = token2.substr(0, breakpoint);
        if (lang.empty() || lang.length() != 2) {
            return false;
        }
        if (token2.find('-') != std::string::npos) {
            std::string country = token2.substr(token2.find('-') + 1, token2.find(';') - token2.find('-') - 1);
            if (country.empty()) {
                return false; // A region code must follow the '-'
            }
        }
        if (checkQValue(token2) == false) {
            return false;
        }
    }
    return true;
}

bool isValidCookie(std::string str) {
    std::vector< std::string > tokens;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, ';')) {
        token.erase(0, token.find_first_not_of(" "));
        tokens.push_back(token);
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        std::string token2 = tokens[i];
        size_t equal = token2.find('=');
        if (equal == std::string::npos || equal == 0 ||
            equal == token2.size() - 1) // the char '=' must be in the middle of the string
            return false;
        std::string key = token2.substr(0, equal);
        if (key.empty()) {
            return false;
        }
        if (checkQValue(token2) == false) {
            return false;
        }
    }
    return true;
}

bool isValidRange(const std::string& str) {
    size_t pos = str.find("bytes=");
    if (pos == std::string::npos) {
        return false;
    }
    std::string range = str.substr(pos + 6);
    std::vector< std::string > tokens;
    std::istringstream iss(range);
    std::string token;
    while (std::getline(iss, token, ',')) {
        token.erase(0, token.find_first_not_of(" "));
        tokens.push_back(token);
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        std::string token2 = tokens[i];
        size_t dash = token2.find('-');
        if (dash == std::string::npos || dash == 0 ||
            dash == token2.size() - 1) // the char '-' must be in the middle of the string
            return false;
        std::string start = token2.substr(0, dash);
        size_t semicolon = token2.find(';');
        std::string end = token2.substr(dash + 1, semicolon - dash - 1);
        if (start.empty() || end.empty()) {
            return false;
        }
        for (size_t j = 0; j < start.size(); j++) {
            if (!isdigit(start[j])) {
                return false;
            }
        }
        for (size_t k = 0; k < end.size(); k++) {
            if (!isdigit(end[k])) {
                return false;
            }
        }
        if (checkQValue(token2) == false) {
            return false;
        }
    }
    return true;
}

bool isValidContentLength(const std::string& str) {
    for (size_t i = 0; i < str.size(); i++) {
        if (!isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

bool specificHeaderValidation(
    // so far, I added these functions, but if we need more, I can add them
    const std::string& key, const std::string& value, ILogger& logger) {

    if (key == "host") {
        if (!isValidHost(value)) {
            logger.log("ERROR", "specificHeaderValidation: Invalid Host header");
            return false;
        }
    }
    if (key == "content-length") {
        if (!isValidContentLength(value)) {
            logger.log("ERROR", "specificHeaderValidation: Invalid Content-Length header");
            return false;
        }
    }
    if (key == "transfer-encoding") {
        const std::string chunked = "chunked";
        if (value.length() < chunked.length() || value.substr(value.length() - chunked.length()) != chunked) {
            logger.log("ERROR", "specificHeaderValidation: Invalid Transfer-Encoding header");
            return false;
        }
    }
    if (key == "connection") { // TODO: should not check only for lower-case
                               // GET / HTTP/1.1
                               // Host: portfolio.com
                               // User-Agent: Wget/1.25.0
                               // Accept: */*
                               // Accept-Encoding: identity
                               // Connection: Keep-Alive
        if (value != "keep-alive" && value != "close") {
            logger.log("ERROR", "specificHeaderValidation: Invalid Connection header");
            return false;
        }
    }
    if (key == "content-type") {
        if (isValidContentType(value) == false) {
            logger.log("ERROR", "specificHeaderValidation: Invalid Content-Type header");
            return false;
        }
    }
    if (key == "accept") {
        if (isValidAccept(value) == false) {
            logger.log("ERROR", "specificHeaderValidation: Invalid Accept header");
            return false;
        }
    }
    if (key == "accept-encoding") {
        if (isValidAcceptEncoding(value) == false) {
            logger.log("ERROR", "specificHeaderValidation: Invalid Accept-Encoding header");
            return false;
        }
    }
    if (key == "accept-language") {
        if (isValidAcceptLanguage(value) == false) {
            logger.log("ERROR", "specificHeaderValidation: Invalid Accept-Language header");
            return false;
        }
    }
    if (key == "cookie") {
        if (isValidCookie(value) == false) {
            logger.log("ERROR", "specificHeaderValidation: Invalid Cookie header");
            return false;
        }
    }
    if (key == "range") {
        if (isValidRange(value) == false) {
            logger.log("ERROR", "specificHeaderValidation: Invalid Range header");
            return false;
        }
    }
    return true;
}
