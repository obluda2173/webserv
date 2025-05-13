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
