#pragma once

#include <fstream>
#include <string>
#include <mutex>
#include <ctime>
#include <sstream>
#include <iostream>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

class Logger {
private:
    std::ofstream logFile_;
    std::mutex mutex_;
    LogLevel minLevel_;
    bool consoleOutput_;
    static Logger instance_;

    Logger();

    static constexpr const char* RESET_COLOR = "\033[0m";
    static constexpr const char* DEBUG_COLOR = "\033[36m";   // 青色
    static constexpr const char* INFO_COLOR = "\033[32m";    // 绿色
    static constexpr const char* WARNING_COLOR = "\033[33m"; // 黄色
    static constexpr const char* ERROR_COLOR = "\033[31m";   // 红色
    static constexpr const char* FATAL_COLOR = "\033[35m";   // 紫色

public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void setLogFile(const std::string& filename);
    void setMinLevel(LogLevel level) { minLevel_ = level; }
    void setConsoleOutput(bool enable) { consoleOutput_ = enable; }
    void log(LogLevel level, const std::string& message);

private:
    std::string getCurrentTimestamp();
    std::string getLevelString(LogLevel level);
    std::string getColorCode(LogLevel level);
};

#define LOG_DEBUG(msg) Logger::getInstance().log(LogLevel::DEBUG, msg)
#define LOG_INFO(msg) Logger::getInstance().log(LogLevel::INFO, msg)
#define LOG_WARNING(msg) Logger::getInstance().log(LogLevel::WARNING, msg)
#define LOG_ERROR(msg) Logger::getInstance().log(LogLevel::ERROR, msg)
#define LOG_FATAL(msg) Logger::getInstance().log(LogLevel::FATAL, msg) 