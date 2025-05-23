#include "CgiHandler.h"
#include "handlerUtils.h"


std::string CgiHandler::_extractQuery(const std::string& uri) {
    const size_t pos = uri.find('?');
    return (pos != std::string::npos) ? uri.substr(pos + 1) : "";
}

std::string CgiHandler::_findInterpreter(std::map< std::string, std::string > cgiMap, const std::string& uri) {
    for (std::map<std::string, std::string>::const_iterator it = cgiMap.begin(); it != cgiMap.end(); ++it) {
        const std::string& ext = it->first;
        const std::string& interpreter = it->second;
        std::string dotExt = "." + ext;

        std::string::size_type pos = uri.find(dotExt);
        if (pos == std::string::npos)
            continue;

        std::string::size_type next = pos + dotExt.size();
        if (next == uri.size() || uri[next] == '/' || uri[next] == '?' || uri[next] == '#') {
            return interpreter;
        }
    }
    return "";
}

std::string CgiHandler::_getPathInfo(std::map< std::string, std::string > cgiMap, const std::string& uri) {
    std::string script = _getScriptName(cgiMap, uri);
    if (script.empty() || script.size() >= uri.size())
        return "";

    std::string::size_type start = script.size();
    std::string::size_type end = uri.find_first_of("?#", start);
    if (end == std::string::npos)
        end = uri.size();

    return uri.substr(start, end - start);
}

std::string CgiHandler::_getScriptName(const std::map<std::string, std::string>& cgiMap, const std::string& uri) {
    std::string interp = _findInterpreter(cgiMap, uri);
    if (interp.empty())
        return "";

    for (std::map<std::string, std::string>::const_iterator it = cgiMap.begin(); it != cgiMap.end(); ++it) {
        const std::string& ext = it->first;
        std::string dotExt = "." + ext;
        std::string::size_type pos = uri.find(dotExt);
        if (pos == std::string::npos)
            continue;

        std::string::size_type afterExt = pos + dotExt.size();
        if (afterExt == uri.size() || uri[afterExt] == '/' || uri[afterExt] == '?' || uri[afterExt] == '#') {
            return uri.substr(0, afterExt);
        }
    }
    return "";
}

std::string CgiHandler::_getRemoteAddr(Connection* conn) {
    struct sockaddr_storage addr = conn->getAddr();
    if (addr.ss_family == AF_INET) {
        sockaddr_in* addr_in = (sockaddr_in*)&addr;
        return getIpv4String(addr_in);
    }
    if (addr.ss_family == AF_INET6) {
        sockaddr_in6* addr_in6 = (sockaddr_in6*)&addr;
        return getIpv6String(*addr_in6);
    }
    return "";
}

std::string CgiHandler::_getServerPort(Connection* conn) {
    struct sockaddr_storage addr = conn->getAddr();
    std::ostringstream ss;
    if (addr.ss_family == AF_INET) {
        sockaddr_in* addr_in = (sockaddr_in*)&addr;
        ss << ntohs(addr_in->sin_port);
        return ss.str();
    }
    if (addr.ss_family == AF_INET6) {
        sockaddr_in6* addr_in6 = (sockaddr_in6*)&addr;
        ss << ntohs(addr_in6->sin6_port);
        return ss.str();
    }
    return "";
}

std::string CgiHandler::_toUpper(const std::string& str) {
    std::string result = str;
    for (char* ptr = &result[0]; ptr < &result[0] + result.size(); ++ptr) {
        *ptr = static_cast< char >(toupper(*ptr));
    }
    return result;
}

std::string CgiHandler::_toLower(const std::string& str) {
    std::string result = str;
    for (char* ptr = &result[0]; ptr < &result[0] + result.size(); ++ptr) {
        *ptr = static_cast< char >(tolower(*ptr));
    }
    return result;
}

void CgiHandler::_replace(std::string& str, char what, char with) {
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == what) {
            str[i] = with;
        }
    }
}

std::string CgiHandler::_trimWhiteSpace(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (std::string::npos == first)
        return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}
