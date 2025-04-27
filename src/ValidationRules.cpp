#include "ValidationRules.h"

ValidationRules::ValidationRules(std::vector<std::string> _locAllowedMethods,
                                 std::vector<std::string> svrAllowedMethods, size_t locClientMaxBody,
                                 size_t _svrClientMaxBody)

    : _locationAllowedMethods(_locAllowedMethods), _serverAllowedMethods(svrAllowedMethods),
      _locClientMaxBody(locClientMaxBody), _svrClientMaxBody(_svrClientMaxBody) {}

ValidationRules::~ValidationRules() {}

const std::vector<std::string>& ValidationRules::getAllowedMethods() {
    return !_locationAllowedMethods.empty() ? _locationAllowedMethods : _serverAllowedMethods;
}

size_t ValidationRules::getClientMaxBodySize() {
    return (_locClientMaxBody != 0) ? _locClientMaxBody : _svrClientMaxBody;
}
