#ifndef HTTPPARSING_H
#define HTTPPARSING_H

#include "ILogger.h"

bool checkValidMethod(const std::string& method);
bool checkValidVersion(const std::string& version);
bool specificHeaderValidation(const std::string& key, const std::string& value, ILogger& logger);
bool isValidRange(const std::string& str);
bool isValidCookie(std::string str);
bool isValidAcceptLanguage(const std::string& str);
bool isValidAcceptEncoding(const std::string& str);
bool isValidAccept(const std::string& str);
bool isValidContentType(const std::string& str);
bool isValidHost(std::string str);
bool checkQValue(std::string str);
bool checkCharsetBoundary(const std::string& str);
bool checkValidVersionSyntax(const std::string& version);
/* used in later stages, not in httpParser anymore */
bool checkMethodImplemented(const std::string& method);
bool checkValidVersion(const std::string& version);
std::string parseOutProtocolAndHost(std::string uri);

#endif // HTTPPARSING_H
