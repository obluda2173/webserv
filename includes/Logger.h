#ifndef LOGGER_H
#define LOGGER_H
#include <string>

class Logger {
    private:
        typedef void (Logger::*t_log_func)(void);
        t_log_func get_debug_level(std::string level);
        void debug(void);
        void info(void);
        void warning(void);
        void error(void);
    public:
        void log(std::string level);
        void log_from(std::string level);
};

#endif // LOGGER_H
