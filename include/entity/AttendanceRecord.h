#pragma once

#include "entity/Date.h"
#include "entity/AttendanceStatus.h"
#include <string>

namespace ams {

/**
 * Immutable value object representing a single attendance entry.
 * Logical identity is defined by the compound key (studentId, date).
 */
struct AttendanceRecord {
    int              studentId{0};
    Date             date{};
    AttendanceStatus status{AttendanceStatus::ABSENT};
    int              teacherId{0};
    std::string      timestamp; ///< ISO 8601 datetime of marking, e.g. "2024-01-15T10:30:00"

    AttendanceRecord() = default;
    AttendanceRecord(int sid, Date d, AttendanceStatus s, int tid, std::string ts);

    std::string toCSVRow() const;
    static AttendanceRecord fromCSVRow(const std::string& line);

    /// Equality based on logical identity: same student, same date
    bool operator==(const AttendanceRecord& o) const noexcept {
        return studentId == o.studentId && date == o.date;
    }
    bool operator!=(const AttendanceRecord& o) const noexcept {
        return !(*this == o);
    }
};

} // namespace ams
