#include "ILogger.h"
#include <sstream>
#include <string>
#include <vector>

bool checkValidMethod(const std::string& method) {
    return method == "GET" || method == "POST" || method == "PUT" || method == "DELETE" || method == "HEAD" ||
           method == "OPTIONS" || method == "PATCH";
}

bool checkValidVersion(const std::string& version) {
    double ver = 0;
    if (version.empty()) {
        return false;
    }
    if (version.find("HTTP/") != 0) {
        return false;
    }
    std::string verStr = version.substr(5);
    if (verStr.find('.') == std::string::npos && verStr.size() != 3) { // TODO: isn't it supposed to be an or "||"
        return false;
    }
    if (!isdigit(verStr[0]) || verStr[1] != '.' || !isdigit(verStr[2])) { // TODO: why check 2 times for a "."?
        return false;
    }
    std::istringstream iss(verStr); // TODO: what's that? can it not just be stringCompare to a valid HTTPVersion,
                                    // is 1.3 a valid Version?
    iss >> ver;
    if (iss.fail() || !iss.eof()) {
        return false;
    }
    if (ver < 0 || ver > 2) {
        return false;
    }
    return true;
}

bool checkCharsetBoundary(
    const std::string& str) { // TODO: unreadable, don't know what it does. Very convoluted, surely bugs
    size_t pos = str.find("charset");
    if (pos != std::string::npos) {
        std::string charset = str.substr(pos + 7);
        size_t equal = charset.find('=');
        if (equal != std::string::npos) {
            charset = charset.substr(equal + 1);
            while (charset[0] == ' ') {
                charset.erase(0, 1);
            }
            if (charset.empty()) {
                return false;
            }
            for (size_t i = 0; i < charset.size(); i++) {
                if (!isalnum(charset[i]) && charset[i] != '-' && charset[i] != '_' && charset[i] != '.') {
                    return false;
                }
            }
        }
    } else {
        return false;
    }
    pos = str.find("boundary");
    if (pos != std::string::npos) {
        std::string boundary = str.substr(pos + 9);
        size_t equal = boundary.find('=');
        if (equal != std::string::npos) {
            boundary = boundary.substr(equal + 1);
            while (boundary[0] == ' ') {
                boundary.erase(0, 1);
            }
            if (boundary.empty()) {
                return false;
            }
            for (size_t i = 0; i < boundary.size(); i++) {
                if (!isalnum(boundary[i]) && boundary[i] != '-' && boundary[i] != '_' && boundary[i] != '.') {
                    return false;
                }
            }
        }
    } else {
        return false;
    }
    return true;
}

bool checkQValue(std::string str) { // TODO: unreadable, don't know what it does. Very convoluted, surely bugs
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == 'q') {
            ++i;
            while (str[i] == ' ') {
                ++i;
            }
            if (str[i] == '=') {
                ++i;
                while (str[i] == ' ') {
                    ++i;
                }
                if (isdigit(str[i])) {
                    std::istringstream iss(str.substr(i, 3));
                    double q = 0;
                    iss >> q;
                    if (iss.fail() || !iss.eof()) {
                        return false;
                    }
                    if (q < 0 || q > 1) {
                        return false;
                    }
                    i += 3;
                } else {
                    return false;
                }
            }
        }
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

bool isValidContentType(const std::string& str) {
    size_t slash = str.find('/');
    if (slash == std::string::npos || slash == 0 ||
        slash == str.size() - 1) // the char '/' must be in the middle of the string
        return false;
    std::string type = str.substr(0, slash);
    if (type != "application" && type != "text" && type != "image" && type != "audio" && type != "video" &&
        type != "multipart" && type != "font" && type != "model" && type != "message" && type != "example") {
        return false;
    }
    if (checkCharsetBoundary(str) == false) {
        return false;
    }
    return true;
}

bool isValidAccept(const std::string& str) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, ',')) {
        tokens.push_back(token);
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        std::string token2 = tokens[i];
        size_t slash = token2.find('/');
        if (slash == std::string::npos || slash == 0 ||
            slash == token2.size() - 1) // the char '/' must be in the middle of the string
            return false;
        std::string type = token2.substr(0, slash);
        if (type != "application" && type != "text" && type != "image" && type != "audio" && type != "video" &&
            type != "multipart" && type != "font" && type != "model" && type != "message" && type != "example") {
            return false;
        }
        if (checkCharsetBoundary(token2) == false) {
            return false;
        }
        if (checkQValue(token2) == false) {
            return false;
        }
    }
    return true;
}

bool isValidAcceptEncoding(const std::string& str) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, ',')) {
        tokens.push_back(token);
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        std::string token2 = tokens[i];
        if (token2 != "gzip" && token2 != "deflate" && token2 != "br" && token2 != "compress" && token2 != "identity") {
            return false;
        }
        if (checkQValue(token2) == false) {
            return false;
        }
    }
    return true;
}

bool isValidAcceptLanguage(const std::string& str) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, ',')) {
        tokens.push_back(token);
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        std::string token2 = tokens[i];
        size_t dash = token2.find('-');
        if (dash != std::string::npos &&
            (dash == 0 || dash == token2.size() - 1)) // the char '-' should be in the middle of the string(if appears)
            return false;
        std::string lang = token2.substr(0, dash);
        if (!lang.empty() && lang.length() != 2) {
            return false;
        }
        if (dash != std::string::npos) {
            std::string region = token2.substr(dash + 1);
            if (region.empty()) {
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
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, ';')) {
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
    std::vector<std::string> tokens;
    std::istringstream iss(range);
    std::string token;
    while (std::getline(iss, token, ',')) {
        tokens.push_back(token);
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        std::string token2 = tokens[i];
        size_t dash = token2.find('-');
        if (dash == std::string::npos || dash == 0 ||
            dash == token2.size() - 1) // the char '-' must be in the middle of the string
            return false;
        std::string start = token2.substr(0, dash);
        std::string end = token2.substr(dash + 1);
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

bool specificHeaderValidation(
    const std::string& key, const std::string& value,
    ILogger* _logger) { // so far, I added these functions, but if we need more, I can add them
    if (key == "Host") {
        if (!isValidHost(value)) {
            _logger->log("ERROR", "specificHeaderValidation: Invalid Host header");
            return false;
        }
    }
    if (key == "Content-Length") {
        std::istringstream iss(value);
        int contentLength;
        iss >> contentLength;
        if (contentLength < 0) {
            _logger->log("ERROR", "specificHeaderValidation: Invalid Content-Length header");
            return false;
        }
    }
    if (key == "Transfer-Encoding") {
        const std::string chunked = "chunked";
        if (value.length() < chunked.length() || value.substr(value.length() - chunked.length()) != chunked) {
            _logger->log("ERROR", "specificHeaderValidation: Invalid Transfer-Encoding header");
            return false;
        }
    }
    if (key == "Connection") {
        if (value != "keep-alive" && value != "close") {
            _logger->log("ERROR", "specificHeaderValidation: Invalid Connection header");
            return false;
        }
    }
    if (key == "Content-Type") {
        if (isValidContentType(value) == false) {
            _logger->log("ERROR", "specificHeaderValidation: Invalid Content-Type header");
            return false;
        }
    }
    if (key == "Accept") {
        if (isValidAccept(value) == false) {
            _logger->log("ERROR", "specificHeaderValidation: Invalid Accept header");
            return false;
        }
    }
    if (key == "Accept-Encoding") {
        if (isValidAcceptEncoding(value) == false) {
            _logger->log("ERROR", "specificHeaderValidation: Invalid Accept-Encoding header");
            return false;
        }
    }
    if (key == "Accept-Language") {
        if (isValidAcceptLanguage(value) == false) {
            _logger->log("ERROR", "specificHeaderValidation: Invalid Accept-Language header");
            return false;
        }
    }
    if (key == "Cookie") {
        if (isValidCookie(value) == false) {
            _logger->log("ERROR", "specificHeaderValidation: Invalid Cookie header");
            return false;
        }
    }
    if (key == "Range") {
        if (isValidRange(value) == false) {
            _logger->log("ERROR", "specificHeaderValidation: Invalid Range header");
            return false;
        }
    }
    return true;
}
