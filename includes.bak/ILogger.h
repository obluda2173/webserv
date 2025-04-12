#ifndef ILOGGER_H
#define ILOGGER_H

#include <string>

class ILogger {
public:
	virtual ~ILogger() = default;
	virtual void log(const std::string& level, const std::string& message) = 0;
};

#endif // ILOGGER_H
