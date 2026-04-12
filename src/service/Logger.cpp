#include "service/Logger.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

namespace ams {

Logger& Logger::instance() {
    static Logger logger; // guaranteed single construction, thread-safe in C++11+
    return logger;
}

void Logger::init(const std::string& path) {
    file_.open(path, std::ios::app);
    if (!file_.is_open()) {
        std::cerr << "[Logger] WARNING: Cannot open log file: " << path
                  << " — logging to stderr only.\n";
    }
    initialized_ = true;
}

void Logger::debug  (const std::string& msg) { log(LogLevel::DEBUG,       msg); }
void Logger::info   (const std::string& msg) { log(LogLevel::INFO,        msg); }
void Logger::warning(const std::string& msg) { log(LogLevel::WARNING,     msg); }
void Logger::error  (const std::string& msg) { log(LogLevel::ERROR_LEVEL, msg); }

void Logger::log(LogLevel level, const std::string& msg) {
    std::string entry =
        "[" + currentTimestamp() + "] [" + levelToString(level) + "] " + msg;

    if (file_.is_open()) {
        file_ << entry << "\n";
        file_.flush();
    }

    if (level == LogLevel::ERROR_LEVEL || level == LogLevel::WARNING) {
        std::cerr << entry << "\n";
    }
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:       return "DEBUG";
        case LogLevel::INFO:        return "INFO ";
        case LogLevel::WARNING:     return "WARN ";
        case LogLevel::ERROR_LEVEL: return "ERROR";
        default:                    return "?????";
    }
}

std::string Logger::currentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm* ptm = std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(ptm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

} // namespace ams
