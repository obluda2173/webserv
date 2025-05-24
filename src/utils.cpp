#include "utils.h"
#include "ConfigStructure.h"
#include "IIONotifier.h"
#include "utils.h"
#include <arpa/inet.h>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void getAddrInfoHelper(const char* node, const char* port, int protocol, struct addrinfo** addrInfo) {
    int status;
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = protocol;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((status = getaddrinfo(node, port, &hints, addrInfo) != 0))
        throw std::runtime_error(std::string("getaddrinfo error: ") + gai_strerror(status));
}

int newSocket(addrinfo* addrInfo) {
    int socketfd = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    if (socketfd < 0) {
        perror("socket");
        exit(1);
    }

    int yes = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    if (bind(socketfd, addrInfo->ai_addr, addrInfo->ai_addrlen) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    return socketfd;
}

int newSocket(std::string ip, std::string port, int protocol) {
    struct addrinfo* addrInfo;
    getAddrInfoHelper(ip.c_str(), port.c_str(), protocol, &addrInfo);
    int socketfd = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    if (socketfd < 0) {
        perror("socket");
        exit(1);
    }

    int yes = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    if (bind(socketfd, addrInfo->ai_addr, addrInfo->ai_addrlen) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(addrInfo);
    return socketfd;
}

int newListeningSocket(addrinfo* addrInfo, int backlog) {
    int socketfd = newSocket(addrInfo);
    if (listen(socketfd, backlog) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    return socketfd;
}

int newListeningSocket(std::string ip, std::string port, int protocol, int backlog) {
    int socketfd = newSocket(ip, port, protocol);
    if (listen(socketfd, backlog) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    return socketfd;
}

int getPort(int socketfd) {
    sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    if (getsockname(socketfd, reinterpret_cast< sockaddr* >(&addr), &addr_len) == -1) {
        std::cerr << "Error getting socket information" << std::endl;
    }

    // Get the port number, noting that network byte order must be converted to host byte order
    return ntohs(addr.sin_port);
}

std::string getAddressAndPort(int socketfd) {
    sockaddr_storage addr;
    socklen_t addr_len = sizeof(addr);
    if (getsockname(socketfd, (sockaddr*)&addr, &addr_len) == -1) {
        std::cerr << "Error getting socket information" << std::endl;
        return "";
    }

    std::string ip;
    if (addr.ss_family == AF_INET)
        ip = getIpv4String((sockaddr_in*)&addr);
    else
        ip = getIpv6String((sockaddr_in6*)&addr);

    // Get port (convert from network to host byte order)
    int port = getPort(socketfd);

    // Combine them into address:port format
    return ip + ":" + to_string(port);
}

std::string toLower(const std::string& str) {
    std::string result = str;
    for (char* ptr = &result[0]; ptr < &result[0] + result.size(); ++ptr) {
        *ptr = static_cast< char >(tolower(*ptr));
    }
    return result;
}

void printTimeval(const timeval& tv) {
    std::cout << "Seconds: " << tv.tv_sec << ", Microseconds: " << tv.tv_usec << std::endl;
}

const char* notifToString(e_notif notif) {
    switch (notif) {
    case CLIENT_HUNG_UP:
        return "CLIENT_HUNG_UP";
    case READY_TO_READ:
        return "READY_TO_READ";
    case READY_TO_WRITE:
        return "READY_TO_WRITE";
    case BROKEN_CONNECTION:
        return "BROKEN_CONNECTION";
    case TIMEOUT:
        return "TIMEOUT";
    default:
        return "UNKNOWN";
    }
}

void printNotif(const t_notif& notif) {
    std::cout << "File Descriptor: " << notif.fd << ", Notification: " << notifToString(notif.notif) << std::endl;
}

std::string getIpv4String(struct sockaddr_in* addr_in) {
    std::stringstream res;
    unsigned char* ip = reinterpret_cast< unsigned char* >(&addr_in->sin_addr.s_addr);
    res << static_cast< int >(ip[0]) << '.' << static_cast< int >(ip[1]) << '.' << static_cast< int >(ip[2]) << '.'
        << static_cast< int >(ip[3]);
    return res.str();
}

std::string getIpv6String(struct sockaddr_in6* addr) {
    std::stringstream res;
    const uint8_t* bytes = addr->sin6_addr.s6_addr;
    for (int i = 0; i < 16; i += 2) {
        res << std::hex << static_cast< int >(bytes[i]) << static_cast< int >(bytes[i + 1]);
        if (i < 14)
            res << ":";
    }
    return res.str();
}

void mustGetAddrInfo(std::string addr, std::string port, struct addrinfo** svrAddrInfo) {
    try {
        getAddrInfoHelper(addr.c_str(), port.c_str(), AF_INET, svrAddrInfo);
    } catch (std::runtime_error& e) {
        try {
            getAddrInfoHelper(addr.c_str(), port.c_str(), AF_INET6, svrAddrInfo);
        } catch (std::runtime_error& e2) {
            std::cerr << "Caught exception: " << e2.what() << std::endl;
            exit(1);
        }
    }
}

void mustTranslateToRealIps(std::vector< ServerConfig >& svrCfgs) {
    for (size_t i = 0; i < svrCfgs.size(); i++) {
        ServerConfig& svrCfg = svrCfgs[i];
        std::set< std::pair< std::string, int > > newListen;
        for (std::set< std::pair< std::string, int > >::iterator it = svrCfg.listen.begin(); it != svrCfg.listen.end();
             it++) {
            std::string ip = it->first;
            std::string port = to_string(it->second);

            struct addrinfo* svrAddrInfo = NULL;
            mustGetAddrInfo(ip, port, &svrAddrInfo);

            if (svrAddrInfo->ai_family == AF_INET)
                ip = getIpv4String((sockaddr_in*)svrAddrInfo->ai_addr);
            else
                ip = getIpv6String((sockaddr_in6*)svrAddrInfo->ai_addr);

            newListen.insert(std::pair< std::string, int >(ip, it->second));
            freeaddrinfo(svrAddrInfo);
        }
        svrCfg.listen = newListen;
    }
}

std::set< std::pair< std::string, std::string > > fillAddrAndPorts(std::vector< ServerConfig > svrCfgs) {
    std::set< std::pair< std::string, std::string > > addrAndPorts;
    for (size_t i = 0; i < svrCfgs.size(); i++) {
        ServerConfig svrCfg = svrCfgs[i];
        for (std::set< std::pair< std::string, int > >::iterator it = svrCfg.listen.begin(); it != svrCfg.listen.end();
             it++) {
            std::string ip = it->first;
            std::string port = to_string(it->second);
            addrAndPorts.insert(std::pair< std::string, std::string >(ip, port));
        }
    }
    return addrAndPorts;
}
