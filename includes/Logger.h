#ifndef LOGGER_H
#define LOGGER_H

#include "ILogger.h"
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

class Logger : public ILogger {
  public:
    enum LEVEL { DEBUG, INFO, WARNING, ERROR };

  private:
    LEVEL _level;
    std::ostream* _cout;
    std::ofstream _file;
    typedef void (Logger::*t_log_func)(const std::string& msg);
    t_log_func get_level_func(std::string level);
    void debug(const std::string& msg);
    void info(const std::string& msg);
    void warning(const std::string& msg);
    void error(const std::string& msg);

  public:
    Logger();
    Logger(std::string filepath);
    ~Logger();

    void setLevel(Logger::LEVEL level);
    void log(const std::string& level, const std::string& msg);
    void log_from(std::string level, const std::string msg);
};

#endif // LOGGER_H
