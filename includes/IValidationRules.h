#ifndef IVALIDATIONRULES_H
#define IVALIDATIONRULES_H

#include <vector>
#include <string>

class IValidationRules {
  public:
    virtual ~IValidationRules() {}
    virtual const std::vector<std::string>& getAllowedMethods() = 0;
    virtual size_t getClientMaxBodySize() = 0;
    // to add more
};

#endif // IVALIDATIONRULES_H
