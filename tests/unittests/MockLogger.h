#ifndef MOCKLOGGER_H
#define MOCKLOGGER_H

#include <string>
#include <tuple>
#include <vector>

class MockLogger {
  private:
    std::vector<std::tuple<std::string, std::string>> _actual_calls;
  public:
    bool logWasCalled(std::vector<std::tuple<std::string, std::string>> calls) {
        if (calls.size() != _actual_calls.size())
            return false;
        for (size_t i = 0; i < calls.size(); i++) {
            if (calls[i] != _actual_calls[i]) {
                return false;
            }
        }
        return true;
    };
    void log(const std::string &level, const std::string &msg) {
        _actual_calls.push_back({level, msg});
    };
};

#endif // MOCKLOGGER_H
