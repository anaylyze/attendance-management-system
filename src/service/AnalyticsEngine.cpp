#include "service/AnalyticsEngine.h"

#include <algorithm>

namespace ams {

StudentAnalytics AnalyticsEngine::computeForStudent(
    const Student&                       student,
    const std::vector<AttendanceRecord>& records,
    double                               lowThreshold)
{
    StudentAnalytics a;
    a.studentId   = student.id();
    a.studentName = student.name();
    a.totalDays   = static_cast<int>(records.size());

    for (const auto& r : records) {
        switch (r.status) {
            case AttendanceStatus::PRESENT: ++a.presentDays; break;
            case AttendanceStatus::ABSENT:  ++a.absentDays;  break;
            case AttendanceStatus::LATE:    ++a.lateDays;    break;
            case AttendanceStatus::EXCUSED: ++a.excusedDays; break;
            default: break;
        }
    }

    const int counted = a.presentDays + a.lateDays + a.excusedDays;
    a.attendancePercent = (a.totalDays > 0)
        ? (static_cast<double>(counted) / static_cast<double>(a.totalDays)) * 100.0
        : 100.0;

    a.lowAttendance = (a.totalDays > 0 && a.attendancePercent < lowThreshold);
    return a;
}

std::vector<StudentAnalytics> AnalyticsEngine::computeAll(
    const StudentRegistry&   registry,
    const AttendanceManager& mgr,
    double                   lowThreshold)
{
    std::vector<StudentAnalytics> result;
    result.reserve(registry.count());

    for (auto it = registry.all().begin(); it != registry.all().end(); ++it) {
        auto records = mgr.recordsForStudent(it->first);
        result.push_back(computeForStudent(it->second, records, lowThreshold));
    }

    std::sort(result.begin(), result.end(),
              [](const StudentAnalytics& a, const StudentAnalytics& b) {
                  return a.attendancePercent < b.attendancePercent;
              });
    return result;
}

std::vector<StudentAnalytics> AnalyticsEngine::getLowAttendance(
    const std::vector<StudentAnalytics>& analytics)
{
    std::vector<StudentAnalytics> result;
    for (const auto& a : analytics) {
        if (a.lowAttendance) result.push_back(a);
    }
    return result;
}

std::map<Date, std::map<AttendanceStatus, int>>
AnalyticsEngine::dailySummary(const AttendanceManager& mgr)
{
    std::map<Date, std::map<AttendanceStatus, int>> summary;
    for (auto it = mgr.allRecords().begin(); it != mgr.allRecords().end(); ++it) {
        for (const auto& r : it->second) {
            summary[it->first][r.status]++;
        }
    }
    return summary;
}

} // namespace ams
