#include "CgiHandler.h"
#include "utils.h"

std::string toUpper(const std::string& str) {
    std::string result = str;
    for (char* ptr = &result[0]; ptr < &result[0] + result.size(); ++ptr) {
        *ptr = static_cast< char >(toupper(*ptr));
    }
    return result;
}

void replace(std::string& str, char what, char with) {
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == what) {
            str[i] = with;
        }
    }
}

std::string trimWhiteSpace(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (std::string::npos == first)
        return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}

std::string extractQuery(const std::string& uri) {
    const size_t pos = uri.find('?');
    return (pos != std::string::npos) ? uri.substr(pos + 1) : "";
}

std::string findInterpreter(std::map< std::string, std::string > cgiMap, const std::string& uri) {
    for (std::map< std::string, std::string >::const_iterator it = cgiMap.begin(); it != cgiMap.end(); ++it) {
        const std::string& ext = it->first;
        const std::string& interpreter = it->second;
        // std::string dotExt = "." + ext;
        std::string dotExt =  ext;

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

std::string getPathInfo(std::map< std::string, std::string > cgiMap, const std::string& uri) {
    std::string script = getScriptName(cgiMap, uri);
    if (script.empty() || script.size() >= uri.size())
        return "";

    std::string::size_type start = script.size();
    std::string::size_type end = uri.find_first_of("?#", start);
    if (end == std::string::npos)
        end = uri.size();

    return uri.substr(start, end - start);
}

std::string getScriptName(const std::map< std::string, std::string >& cgiMap, const std::string& uri) {
    std::string interp = findInterpreter(cgiMap, uri);
    if (interp.empty())
        return "";

    for (std::map< std::string, std::string >::const_iterator it = cgiMap.begin(); it != cgiMap.end(); ++it) {
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

std::string getRemoteAddr(Connection* conn) {
    struct sockaddr_storage addr = conn->getAddr();
    if (addr.ss_family == AF_INET) {
        sockaddr_in* addr_in = (sockaddr_in*)&addr;
        return getIpv4String(addr_in);
    }
    if (addr.ss_family == AF_INET6) {
        sockaddr_in6* addr_in6 = (sockaddr_in6*)&addr;
        return getIpv6String(addr_in6);
    }
    return "";
}

std::string getServerPort(Connection* conn) {
    std::string rawPort = conn->getAddrPort();
    size_t pos = rawPort.rfind(":");
    return (pos == std::string::npos ? rawPort : rawPort.substr(pos + 1));
}
