#include "Logger.h"
#include <iostream>

Logger::t_log_func Logger::get_debug_level(std::string level) {
    if (level == "DEBUG") { return &Logger::debug; }
    if (level == "INFO") { return &Logger::info; }
    if (level == "WARNING") { return &Logger::warning; }
    if (level == "ERROR") { return &Logger::error; }
    return NULL;
}

void Logger::log(std::string level) {
    Logger::t_log_func log_func= get_debug_level(level);
    if (log_func)
        (this->*log_func)();
    else
        std::cout << "[ Probably complaining about insignificant problems ]" << std::endl;
    return;
}

void Logger::debug() {
    std::cout << "[DEBUG]" << std::endl << "I love having extra bacon for my 7XL-double-cheese-triple-pickle-special ketchup burger. I really do!" << std::endl;
}

void Logger::info() {
    std::cout << "[INFO]" << std::endl << "I cannot believe adding extra bacon costs more money. You didn’t put enough bacon in my burger! If you did, I wouldn’t be asking for more!" << std::endl;
}
void Logger::warning() {
    std::cout << "[WARNING]" << std::endl << "I think I deserve to have some extra bacon for free. I’ve been coming for years, whereas you started working here just last month."<< std::endl;
}

void Logger::error() {
    std::cout << "[ERROR]" << std::endl << "This is unacceptable! I want to speak to the manager now." << std::endl;
}

void Logger::log_from(std::string level) {
    std::string levels[4] = {"DEBUG", "INFO", "WARNING", "ERROR"};
    int level_nbr = -1;
    for (int i = 0; i < 4 ; i++)
        if (level == levels[i])
            level_nbr = i;

    switch (level_nbr) {
    case (0):
        debug();
        // fall through
    case (1):
        info();
        // fall through
    case (2):
        warning();
        // fall through
    case (3):
        error();
        break;
    default:
        std::cout << "[ Probably complaining about insignificant problems ]" << std::endl;
    }
}
