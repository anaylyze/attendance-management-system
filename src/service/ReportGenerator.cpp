#include "service/ReportGenerator.h"
#include "service/Logger.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <sys/stat.h>
#include <ctime>
#include <chrono>

#ifdef _WIN32
#  include <direct.h>
#  define AMS_MKDIR(p) _mkdir(p)
#else
#  include <sys/types.h>
#  define AMS_MKDIR(p) mkdir((p), 0755)
#endif

namespace ams {

ReportGenerator::ReportGenerator(std::string reportsDir)
    : reportsDir_{std::move(reportsDir)}
{}

void ReportGenerator::ensureReportsDir() {
    AMS_MKDIR(reportsDir_.c_str());
}

std::string ReportGenerator::projectSummaryPath()  const { return reportsDir_ + "/summary.txt";               }
std::string ReportGenerator::analyticsReportPath() const { return reportsDir_ + "/analytics_report.txt";      }
std::string ReportGenerator::lowAttendancePath()   const { return reportsDir_ + "/low_attendance_report.txt"; }

// ─────────────────────────────────────────────────────────────────────────────
// Summary (Architecture Doc)
// ─────────────────────────────────────────────────────────────────────────────

void ReportGenerator::generateProjectSummary() {
    ensureReportsDir();
    std::ofstream f(projectSummaryPath());
    if (!f.is_open()) throw std::runtime_error("Cannot open: " + projectSummaryPath());

    f << "============================================================\n"
      << "  ATTENDANCE MANAGEMENT SYSTEM - PROJECT SUMMARY\n"
      << "============================================================\n\n"

      << "SYSTEM OVERVIEW\n"
      << "---------------\n"
      << "A terminal-based, multi-student attendance management system\n"
      << "written in standard C++17 with no external dependencies.\n"
      << "Designed for 50-5000 students with clean layered architecture.\n\n"

      << "============================================================\n"
      << "ARCHITECTURE - LAYERED DESIGN (5 Layers)\n"
      << "============================================================\n\n"
      << "  Layer 1  ENTITY\n"
      << "           Core domain objects and value types.\n"
      << "           Classes: Date, Person (abstract), Student, Teacher,\n"
      << "                    AttendanceRecord, Comment, AttendanceStatus\n\n"
      << "  Layer 2  COMMAND\n"
      << "           Encapsulates reversible mutations.\n"
      << "           Classes: ICommand (interface), MarkAttendanceCommand,\n"
      << "                    EditAttendanceCommand\n\n"
      << "  Layer 3  MANAGER\n"
      << "           Business logic and state ownership.\n"
      << "           Classes: StudentRegistry, AttendanceManager\n\n"
      << "  Layer 4  SERVICE\n"
      << "           Cross-cutting infrastructure.\n"
      << "           Classes: Logger, FileManager, UndoRedoStack,\n"
      << "                    AnalyticsEngine, ReportGenerator\n\n"
      << "  Layer 5  UI\n"
      << "           Terminal I/O only. No business logic.\n"
      << "           Classes: MenuController\n\n"

      << "============================================================\n"
      << "DESIGN PATTERNS\n"
      << "============================================================\n\n"
      << "  Command Pattern   UndoRedoStack manages ICommand objects.\n"
      << "                    execute() pushes to undo stack.\n"
      << "                    undo() pops undo, pushes to redo.\n"
      << "                    Any new command clears redo stack.\n\n"
      << "  Singleton         Logger - single log sink per process.\n\n"
      << "  Template Method   Person provides identity contract;\n"
      << "                    Student/Teacher implement role() + toCSVRow().\n\n"
      << "  Repository        StudentRegistry with query methods.\n\n"
      << "  Facade            FileManager hides all I/O complexity.\n\n"

      << "============================================================\n"
      << "DATA STRUCTURES - JUSTIFIED\n"
      << "============================================================\n\n"
      << "  std::map<int, Student>   (StudentRegistry)\n"
      << "    WHY: O(log n) ordered lookup; rbegin() gives max ID in O(1).\n\n"
      << "  std::unordered_map<Date, vector<AttendanceRecord>>\n"
      << "    WHY: O(1) date lookup; custom hash via YYYYMMDD integer pack.\n\n"
      << "  std::stack<unique_ptr<ICommand>> x2  (UndoRedoStack)\n"
      << "    WHY: LIFO = natural reversal; unique_ptr = no leaks.\n\n"
      << "  std::vector<Comment>  (flat list, filtered on read)\n"
      << "    WHY: Comments are rare; linear scan is negligible.\n\n"

      << "============================================================\n"
      << "FILE I/O DESIGN\n"
      << "============================================================\n\n"
      << "  Format: CSV with fixed headers.\n"
      << "  data/students.csv    id,name,role,section,enrollmentYear,rollNumber\n"
      << "  data/attendance.csv  studentId,date,status,teacherId,timestamp\n"
      << "  data/comments.csv    studentId,date,authorId,text,timestamp\n"
      << "  data/system.log      Timestamped action log\n\n"
      << "  Corruption Handling:\n"
      << "    Each row parsed in isolated try/catch - bad rows skipped.\n"
      << "    Column count validated before type conversion.\n\n"

      << "============================================================\n"
      << "KEY CHALLENGES\n"
      << "============================================================\n\n"
      << "  1. Include Cycle: AttendanceManager <-> Commands\n"
      << "     Solved by forward-declaring AttendanceManager in command headers.\n\n"
      << "  2. Safe Undo Ownership Transfer\n"
      << "     Call undo() BEFORE std::move() to avoid dangling pointer on throw.\n\n"
      << "  3. Per-Row CSV Corruption Isolation\n"
      << "     One bad row must not abort the entire load operation.\n\n"
      << "  4. Date as unordered_map Key\n"
      << "     Specialised std::hash<Date> using YYYYMMDD integer packing.\n\n"
      << "============================================================\n";

    Logger::instance().info("Generated project summary: " + projectSummaryPath());
}

// ─────────────────────────────────────────────────────────────────────────────
// Analytics Report
// ─────────────────────────────────────────────────────────────────────────────

void ReportGenerator::generateAnalyticsReport(
    const std::vector<StudentAnalytics>& analytics,
    const StudentRegistry& /*registry*/)
{
    ensureReportsDir();
    std::ofstream f(analyticsReportPath());
    if (!f.is_open()) throw std::runtime_error("Cannot open: " + analyticsReportPath());

    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm* ptm = std::localtime(&t);

    f << "============================================================\n"
      << "  ATTENDANCE ANALYTICS REPORT\n"
      << "  Generated: " << std::put_time(ptm, "%Y-%m-%d %H:%M:%S") << "\n"
      << "============================================================\n\n";

    f << std::left
      << std::setw(6)  << "ID"
      << std::setw(22) << "Name"
      << std::setw(7)  << "Total"
      << std::setw(7)  << "Pres."
      << std::setw(7)  << "Abs."
      << std::setw(7)  << "Late"
      << std::setw(7)  << "Exc."
      << std::setw(9)  << "Att. %"
      << "Status\n";
    f << std::string(72, '-') << "\n";

    int lowCount = 0;
    for (const auto& a : analytics) {
        if (a.lowAttendance) ++lowCount;
        f << std::left
          << std::setw(6)  << a.studentId
          << std::setw(22) << a.studentName.substr(0, 21)
          << std::setw(7)  << a.totalDays
          << std::setw(7)  << a.presentDays
          << std::setw(7)  << a.absentDays
          << std::setw(7)  << a.lateDays
          << std::setw(7)  << a.excusedDays
          << std::setw(9)  << std::fixed << std::setprecision(1) << a.attendancePercent
          << (a.lowAttendance ? "LOW ATTENDANCE" : "OK") << "\n";
    }

    f << "\n--- SUMMARY ---\n"
      << "Total students       : " << analytics.size() << "\n"
      << "Low attendance (<75%): " << lowCount << "\n";

    Logger::instance().info("Generated analytics report: " + analyticsReportPath());
}

// ─────────────────────────────────────────────────────────────────────────────
// Low Attendance Report
// ─────────────────────────────────────────────────────────────────────────────

void ReportGenerator::generateLowAttendanceReport(
    const std::vector<StudentAnalytics>& analytics)
{
    ensureReportsDir();
    std::ofstream f(lowAttendancePath());
    if (!f.is_open()) throw std::runtime_error("Cannot open: " + lowAttendancePath());

    f << "============================================================\n"
      << "  LOW ATTENDANCE REPORT (Threshold: < 75%)\n"
      << "============================================================\n\n";

    auto lowStudents = AnalyticsEngine::getLowAttendance(analytics);

    if (lowStudents.empty()) {
        f << "All students meet the 75% attendance requirement.\n";
    } else {
        for (const auto& a : lowStudents) {
            f << "Student ID    : " << a.studentId << "\n"
              << "Name          : " << a.studentName << "\n"
              << "Attendance    : " << std::fixed << std::setprecision(2)
                                   << a.attendancePercent << "%\n"
              << "Present Days  : " << a.presentDays  << "\n"
              << "Late Days     : " << a.lateDays     << "\n"
              << "Excused Days  : " << a.excusedDays  << "\n"
              << "Absent Days   : " << a.absentDays   << " / " << a.totalDays << " total\n"
              << std::string(44, '-') << "\n";
        }
        double pct = (analytics.empty()) ? 0.0
            : 100.0 * static_cast<double>(lowStudents.size()) /
                      static_cast<double>(analytics.size());
        f << "\nSTUDENTS AT RISK: " << lowStudents.size() << " of "
          << analytics.size() << " (" << std::fixed << std::setprecision(1)
          << pct << "% of cohort)\n";
    }

    Logger::instance().info("Generated low-attendance report: " + lowAttendancePath());
}

} // namespace ams
