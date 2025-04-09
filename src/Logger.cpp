#include "Logger.h"
#include <iostream>

Logger::t_log_func Logger::get_debug_level(std::string level) {
    if (level == "DEBUG") { return &Logger::debug; }
    if (level == "INFO") { return &Logger::info; }
    if (level == "WARNING") { return &Logger::warning; }
    if (level == "ERROR") { return &Logger::error; }
    return NULL;
}

void Logger::log(std::string level, const std::string& message) {
    Logger::t_log_func log_func = get_debug_level(level);
    if (log_func)
        (this->*log_func)(message);
    else
        std::cout << "[ Probably complaining about insignificant problems ] " << message << std::endl;
}

void Logger::debug(const std::string& msg) {
    std::cout << "[DEBUG] " << msg << std::endl;
}

void Logger::info(const std::string& msg) {
    std::cout << "[INFO] " << msg << std::endl;
}

void Logger::warning(const std::string& msg) {
    std::cout << "[WARNING] " << msg << std::endl;
}

void Logger::error(const std::string& msg) {
    std::cout << "[ERROR] " << msg << std::endl;
}

void Logger::log_from(std::string level, const std::string msg) {
    std::string levels[4] = {"DEBUG", "INFO", "WARNING", "ERROR"};
    int level_nbr = -1;
    for (int i = 0; i < 4 ; i++)
        if (level == levels[i])
            level_nbr = i;

    switch (level_nbr) {
    case (0):
        debug(msg);
        // fall through
    case (1):
        info(msg);
        // fall through
    case (2):
        warning(msg);
        // fall through
    case (3):
        error(msg);
        break;
    default:
        std::cout << "[ Probably complaining about insignificant problems ]" << std::endl;
    }
}
