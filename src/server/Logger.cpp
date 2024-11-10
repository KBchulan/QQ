#include "Logger.h"
#include <iostream>

Logger::Logger() {
    setLogFile("server.log");
}

void Logger::setLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (logFile_.is_open()) {
        logFile_.close();
    }
    logFile_.open(filename, std::ios::app);
}

std::string Logger::getColorCode(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "\033[36m";    // 青色
        case LogLevel::INFO: return "\033[32m";     // 绿色
        case LogLevel::WARNING: return "\033[33m";  // 黄色
        case LogLevel::ERROR: return "\033[31m";    // 红色
        default: return "";
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string timestamp = getCurrentTimestamp();
    std::string levelStr = getLevelString(level);
    std::string colorCode = getColorCode(level);
    
    // 写入文件（无颜色）
    logFile_ << timestamp << " [" << levelStr << "] " << message << std::endl;
    logFile_.flush();
    
    // 输出到控制台（带颜色）
    std::cout << colorCode << timestamp << " [" << levelStr << "] " 
              << message << RESET_COLOR << std::endl;
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
        default: return "UNKNOWN";
    }
} 