#ifndef LOGGER_H
#define LOGGER_H

#include "ILogger.h"
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

class Logger : public ILogger {
  private:
    std::ostream* _cout;
    std::ofstream _file;
    typedef void (Logger::*t_log_func)(const std::string& msg);
    t_log_func get_debug_level(std::string level);
    void debug(const std::string& msg);
    void info(const std::string& msg);
    void warning(const std::string& msg);
    void error(const std::string& msg);

  public:
    Logger();
    Logger(std::string filepath);
    ~Logger();

    void log(const std::string& level, const std::string& msg);
    void log_from(std::string level, const std::string msg);
};

#endif // LOGGER_H
