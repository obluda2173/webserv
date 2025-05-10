#include "handlerUtils.h"

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
                result += static_cast<char>((hi << 4) | lo);
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
    std::vector<std::string> segments;
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
