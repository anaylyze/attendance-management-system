#pragma once

#include "manager/StudentRegistry.h"
#include "manager/AttendanceManager.h"
#include <string>
#include <vector>
#include <map>

namespace ams {

/// Per-student attendance analytics snapshot.
struct StudentAnalytics {
    int         studentId{0};
    std::string studentName;
    int         totalDays{0};
    int         presentDays{0};  ///< Status == PRESENT
    int         absentDays{0};   ///< Status == ABSENT
    int         lateDays{0};     ///< Status == LATE
    int         excusedDays{0};  ///< Status == EXCUSED
    double      attendancePercent{100.0}; ///< (present+late+excused)/total * 100
    bool        lowAttendance{false};     ///< true if percent < threshold
};

/**
 * Stateless, pure analytics engine.
 * All methods are static — no mutable state, no side effects.
 * Designed for easy unit testing.
 *
 * Attendance percentage formula:
 *   (PRESENT + LATE + EXCUSED) / totalDays * 100
 *   ABSENT is the only non-contributing status.
 *   Students with zero records are treated as 100% (not yet assessed).
 */
class AnalyticsEngine {
public:
    static constexpr double DEFAULT_LOW_THRESHOLD = 75.0;

    /// Compute analytics for a single student given their records.
    static StudentAnalytics computeForStudent(
        const Student&                       student,
        const std::vector<AttendanceRecord>& records,
        double                               lowThreshold = DEFAULT_LOW_THRESHOLD);

    /**
     * Compute analytics for all registered students.
     * Result is sorted by attendancePercent ascending (worst first) so
     * problem cases appear at the top of any report.
     */
    static std::vector<StudentAnalytics> computeAll(
        const StudentRegistry&   registry,
        const AttendanceManager& mgr,
        double                   lowThreshold = DEFAULT_LOW_THRESHOLD);

    /// Filter a computeAll result to only low-attendance students.
    static std::vector<StudentAnalytics> getLowAttendance(
        const std::vector<StudentAnalytics>& analytics);

    /// Build a per-date summary: date → {status → count}
    static std::map<Date, std::map<AttendanceStatus, int>> dailySummary(
        const AttendanceManager& mgr);
};

} // namespace ams
