#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>

namespace ams {
namespace util {

/// Returns current local time as "YYYY-MM-DDTHH:MM:SS"
inline std::string currentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm* ptm = std::localtime(&t); // single-threaded: localtime is safe
    std::ostringstream oss;
    oss << std::put_time(ptm, "%Y-%m-%dT%H:%M:%S");
    return oss.str();
}

/// Trim leading/trailing whitespace from a string
inline std::string trim(const std::string& s) {
    const auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return {};
    const auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

} // namespace util
} // namespace ams
