#ifndef VALIDATIONRULES_H
#define VALIDATIONRULES_H

#include "ConfigStructure.h"
#include "IValidationRules.h"

class ValidationRules : public IValidationRules {
  private:
    const ServerConfig& _server;
    const LocationConfig* _location;

  public:
    ValidationRules(const ServerConfig& server, const LocationConfig* location);
    ~ValidationRules();
    const std::vector<std::string>& getAllowedMethods();
    size_t getClientMaxBodySize();
    // to add more
};

#endif // VALIDATIONRULES_H
