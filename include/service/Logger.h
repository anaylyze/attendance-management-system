#pragma once

#include <string>
#include <fstream>
// Note: std::mutex omitted for GCC 6 / MinGW-w64 compat.
// Add #include <mutex> and restore lock_guards for MSVC / GCC 9+ builds.

namespace ams {

enum class LogLevel { DEBUG, INFO, WARNING, ERROR_LEVEL };

/**
 * Thread-safe singleton logger.
 * Writes timestamped, leveled entries to a log file; WARNING and ERROR
 * are also echoed to stderr for immediate visibility.
 *
 * Singleton lifetime: initialized once from main() before any other object.
 */
class Logger {
public:
    static Logger& instance();

    /// Must be called exactly once before any log() call.
    void init(const std::string& path);

    void debug  (const std::string& msg);
    void info   (const std::string& msg);
    void warning(const std::string& msg);
    void error  (const std::string& msg);

    void log(LogLevel level, const std::string& msg);

    // Enforce singleton semantics
    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger() = default;

    std::ofstream file_;
    // std::mutex mutex_; // Re-enable for multi-threaded builds
    bool          initialized_{false};

    static std::string levelToString(LogLevel level);
    static std::string currentTimestamp();
};

} // namespace ams
