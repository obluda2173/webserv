#include "test_main.h"

std::string buildUri(std::string script,
                     std::vector< std::pair< std::string, std::vector< std::string > > > queryParams) {
    std::string queryString = "/" + script + "?";
    size_t count = 0;
    for (std::vector< std::pair< std::string, std::vector< std::string > > >::iterator it = queryParams.begin();
         it != queryParams.end(); it++) {
        std::string key = it->first;
        for (size_t i = 0; i < it->second.size(); i++) {
            queryString += it->first + "=" + it->second[i];
            if (i + 1 < it->second.size())
                queryString += "&";
        }

        if ((count + 1) != queryParams.size())
            queryString += "&";
        count++;
    }
    return queryString;
}

std::string getOutput(Connection* conn) {
    
    std::string gotOutput = "";
    std::vector< char > buffer(1024);
    size_t r = 0;
    while (!conn->_response.body->isDone()) {
        r = conn->_response.body->read(buffer.data(), 1024);
        gotOutput += std::string(buffer.data(), r);
    }
    return gotOutput;
}

sockaddr_storage createIPv4Address(const char* ip, uint16_t port) {
    sockaddr_storage storage{};
    sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(&storage);
    
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr->sin_addr);
    
    return storage;
}

void printAddress(const sockaddr_storage& addr) {
    char ipstr[INET_ADDRSTRLEN];
    
    if (addr.ss_family == AF_INET) {
        const sockaddr_in* a = reinterpret_cast<const sockaddr_in*>(&addr);
        inet_ntop(AF_INET, &a->sin_addr, ipstr, sizeof(ipstr));
        std::cout << "IPv4: " << ipstr << ":" << ntohs(a->sin_port) << "\n";
    }
}
