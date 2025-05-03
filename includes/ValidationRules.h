#ifndef VALIDATIONRULES_H
#define VALIDATIONRULES_H

#include "ConfigStructure.h"
#include "IValidationRules.h"

class ValidationRules : public IValidationRules {
  private:
    std::vector<std::string> _locationAllowedMethods;
    std::vector<std::string> _serverAllowedMethods;
    size_t _locClientMaxBody;
    size_t _svrClientMaxBody;

  public:
    ValidationRules(std::vector<std::string> _locAllowedMethods, std::vector<std::string> svrAllowedMethods,
                    size_t locClientMaxBody, size_t _svrClientMaxBody);
    ~ValidationRules();
    const std::vector<std::string>& getAllowedMethods();
    size_t getClientMaxBodySize();
    // to add more
};

class ValidationRulesFactory {
  public:
    ValidationRules build(const ServerConfig& server, const LocationConfig& location) {
        ValidationRules rules(location.common.allowMethods, server.common.allowMethods, location.common.clientMaxBody,
                              server.common.clientMaxBody);
        return rules;
    }
};

#endif // VALIDATIONRULES_H
