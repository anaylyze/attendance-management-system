#pragma once

#include "service/AnalyticsEngine.h"
#include "manager/StudentRegistry.h"
#include <string>
#include <vector>

namespace ams {

/**
 * Generates human-readable text reports to disk.
 * All report paths are rooted at reportsDir_.
 *
 * Reports generated:
 *   summary.txt              — Project architecture & design decisions
 *   analytics_report.txt     — Full per-student attendance table
 *   low_attendance_report.txt — Students below the 75% threshold
 */
class ReportGenerator {
public:
    explicit ReportGenerator(std::string reportsDir);

    void ensureReportsDir();

    /**
     * Generates summary.txt: project architecture, OOP design, data structures,
     * file I/O design, scalability notes, and key engineering challenges.
     */
    void generateProjectSummary();

    /**
     * Generates analytics_report.txt from a pre-computed analytics vector.
     * Accepts the registry for name lookups.
     */
    void generateAnalyticsReport(const std::vector<StudentAnalytics>& analytics,
                                 const StudentRegistry&                registry);

    /**
     * Generates low_attendance_report.txt for students below threshold.
     */
    void generateLowAttendanceReport(const std::vector<StudentAnalytics>& analytics);

    const std::string& reportsDir() const noexcept { return reportsDir_; }

private:
    std::string reportsDir_;

    std::string projectSummaryPath()    const;
    std::string analyticsReportPath()   const;
    std::string lowAttendancePath()     const;
};

} // namespace ams
