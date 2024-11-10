#pragma once

#include <fstream>
#include <string>
#include <mutex>
#include <ctime>
#include <sstream>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
private:
    std::ofstream logFile_;
    std::mutex mutex_;
    static Logger instance_;

    Logger();

public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void log(LogLevel level, const std::string& message);
    void setLogFile(const std::string& filename);

private:
    std::string getCurrentTimestamp();
    std::string getLevelString(LogLevel level);
    std::string getColorCode(LogLevel level);
    static constexpr const char* RESET_COLOR = "\033[0m";
};

#define LOG_DEBUG(msg) Logger::getInstance().log(LogLevel::DEBUG, msg)
#define LOG_INFO(msg) Logger::getInstance().log(LogLevel::INFO, msg)
#define LOG_WARNING(msg) Logger::getInstance().log(LogLevel::WARNING, msg)
#define LOG_ERROR(msg) Logger::getInstance().log(LogLevel::ERROR, msg) 