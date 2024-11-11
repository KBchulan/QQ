#include "Logger.h"
#include <iostream>

Logger::Logger() : minLevel_(LogLevel::DEBUG), consoleOutput_(true) {}

void Logger::setLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (logFile_.is_open()) {
        logFile_.close();
    }
    logFile_.open(filename, std::ios::app);
}

std::string Logger::getColorCode(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return DEBUG_COLOR;
        case LogLevel::INFO: return INFO_COLOR;
        case LogLevel::WARNING: return WARNING_COLOR;
        case LogLevel::ERROR: return ERROR_COLOR;
        case LogLevel::FATAL: return FATAL_COLOR;
        default: return "";
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < minLevel_) return;

    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string timestamp = getCurrentTimestamp();
    std::string levelStr = getLevelString(level);
    std::string colorCode = getColorCode(level);
    
    std::string logMessage = timestamp + " [" + levelStr + "] " + message;
    
    // 写入文件
    if (logFile_.is_open()) {
        logFile_ << logMessage << std::endl;
        logFile_.flush();
    }
    
    // 输出到控制台
    if (consoleOutput_) {
        std::cout << colorCode << logMessage << RESET_COLOR << std::endl;
    }
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::time(nullptr);
    auto* timeinfo = std::localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return std::string(buffer);
}

std::string Logger::getLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
} 