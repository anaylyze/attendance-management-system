#pragma once

#include <string>
#include <stdexcept>
#include <cstdint>

namespace ams {

/// Strongly-typed enum; uint8_t backing for storage efficiency in large datasets.
enum class AttendanceStatus : uint8_t {
    PRESENT = 0,
    ABSENT  = 1,
    LATE    = 2,
    EXCUSED = 3
};

inline std::string statusToString(AttendanceStatus s) {
    switch (s) {
        case AttendanceStatus::PRESENT: return "PRESENT";
        case AttendanceStatus::ABSENT:  return "ABSENT";
        case AttendanceStatus::LATE:    return "LATE";
        case AttendanceStatus::EXCUSED: return "EXCUSED";
        default:                        return "UNKNOWN";
    }
}

inline AttendanceStatus stringToStatus(const std::string& s) {
    if (s == "PRESENT") return AttendanceStatus::PRESENT;
    if (s == "ABSENT")  return AttendanceStatus::ABSENT;
    if (s == "LATE")    return AttendanceStatus::LATE;
    if (s == "EXCUSED") return AttendanceStatus::EXCUSED;
    throw std::invalid_argument("Unknown attendance status: '" + s + "'");
}

} // namespace ams
