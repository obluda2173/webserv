#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include "_Colors.h"

class Logger {
    private:
        typedef void (Logger::*t_log_func)(const std::string& msg);
        t_log_func get_debug_level(std::string level);
        void debug(const std::string& msg);
        void info(const std::string& msg);
        void warning(const std::string& msg);
        void error(const std::string& msg);

    public:
        void log(std::string level, const std::string& msg);
        void log_from(std::string level, const std::string msg);
};

#endif // LOGGER_H
