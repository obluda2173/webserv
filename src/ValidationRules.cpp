#include "ValidationRules.h"

ValidationRules::ValidationRules(const ServerConfig& server, const LocationConfig* location)
    : _server(server), _location(location) {}

ValidationRules::~ValidationRules() {}

const std::vector<std::string>& ValidationRules::getAllowedMethods() {
    return (_location && !_location->common.allowMethods.empty())
        ? _location->common.allowMethods
        : _server.common.allowMethods;
}

size_t ValidationRules::getClientMaxBodySize() {
    return (_location && _location->common.clientMaxBody != 0)
        ? _location->common.clientMaxBody
        : _server.common.clientMaxBody;
}