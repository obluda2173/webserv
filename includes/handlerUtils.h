#ifndef HANDLERUTILS_H
#define HANDLERUTILS_H

#include <string>
#include <vector>
#include <sstream>

#include "Router.h"
#include "Connection.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

#define DEFAULT_HTTP_VERSION "HTTP/1.1"
#define DEFAULT_CONTENT_LANGUAGE "en-US"

int hexToInt(char c);
std::string decodePercent(const std::string& str);
std::string normalizePath(const std::string& root, const std::string& uri);
void setResponse(HttpResponse& resp, int statusCode, const std::string& statusMessage, const std::string& contentType, size_t contentLength, IBodyProvider* bodyProvider);


#endif // HANDLERUTILS_H