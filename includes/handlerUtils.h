#ifndef HANDLERUTILS_H
#define HANDLERUTILS_H

#include <string>
#include <vector>
#include <sstream>

int hexToInt(char c);
std::string decodePercent(const std::string& str);
std::string normalizePath(const std::string& root, const std::string& uri);

#endif // HANDLERUTILS_H