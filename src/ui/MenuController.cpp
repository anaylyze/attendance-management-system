#include "ui/MenuController.h"
#include "service/AnalyticsEngine.h"
#include "service/Logger.h"
#include "util/Utils.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <limits>
#include <stdexcept>

namespace ams {

MenuController::MenuController(StudentRegistry&   registry,
                               AttendanceManager& attendance,
                               FileManager&       fileManager,
                               ReportGenerator&   reportGen)
    : registry_{registry},
      attendance_{attendance},
      fileManager_{fileManager},
      reportGen_{reportGen}
{}

// ──────────────────────────────────────────────────────────────────────────────
// Main Event Loop
// ──────────────────────────────────────────────────────────────────────────────

void MenuController::run() {
    printHeader("ATTENDANCE MANAGEMENT SYSTEM  v1.0");
    std::cout << "  Loading persistent data...\n";

    try {
        fileManager_.loadAll(registry_, attendance_);
        std::cout << "  Loaded " << registry_.count() << " student(s).\n";
    } catch (const std::exception& e) {
        Logger::instance().error(std::string("Startup load failed: ") + e.what());
        std::cout << "  WARNING: Could not load data — starting fresh. (" << e.what() << ")\n";
    }

    pressEnterToContinue();

    while (running_) {
        showMainMenu();
    }

    // Auto-save on exit
    std::cout << "\n  Saving data...\n";
    try {
        fileManager_.saveAll(registry_, attendance_);
        std::cout << "  Data saved successfully.\n";
    } catch (const std::exception& e) {
        Logger::instance().error(std::string("Exit save failed: ") + e.what());
        std::cout << "  ERROR: Data could not be saved: " << e.what() << "\n";
    }
    std::cout << "\n  Goodbye.\n";
}

// ──────────────────────────────────────────────────────────────────────────────
// Top-Level Menus
// ──────────────────────────────────────────────────────────────────────────────

void MenuController::showMainMenu() {
    printHeader("MAIN MENU");
    std::cout
        << "  [1] Student Management\n"
        << "  [2] Attendance Management\n"
        << "  [3] Reports & Analytics\n"
        << "  [4] Undo / Redo\n"
        << "  [5] Save Data Now\n"
        << "  [0] Exit\n";
    printSeparator('-');

    const int choice = readInt("  > ", 0, 5);
    switch (choice) {
        case 1: handleStudentMenu();    break;
        case 2: handleAttendanceMenu(); break;
        case 3: handleReportMenu();     break;
        case 4: handleUndoRedo();       break;
        case 5:
            try {
                fileManager_.saveAll(registry_, attendance_);
                std::cout << "\n  [OK] Data saved.\n";
                Logger::instance().info("Manual save triggered");
            } catch (const std::exception& e) {
                std::cout << "\n  [ERR] Save failed: " << e.what() << "\n";
            }
            pressEnterToContinue();
            break;
        case 0:
            running_ = false;
            break;
    }
}

void MenuController::handleStudentMenu() {
    while (true) {
        printHeader("STUDENT MANAGEMENT");
        std::cout
            << "  [1] Add Student\n"
            << "  [2] Remove Student\n"
            << "  [3] List All Students\n"
            << "  [4] Search by Name\n"
            << "  [0] Back\n";
        printSeparator('-');

        const int choice = readInt("  > ", 0, 4);
        if (choice == 0) break;
        switch (choice) {
            case 1: addStudent();    break;
            case 2: removeStudent(); break;
            case 3: listStudents();  break;
            case 4: searchStudent(); break;
        }
    }
}

void MenuController::handleAttendanceMenu() {
    while (true) {
        printHeader("ATTENDANCE MANAGEMENT");
        std::cout
            << "  [1] Mark Attendance\n"
            << "  [2] Edit Attendance\n"
            << "  [3] View by Date\n"
            << "  [4] View by Student\n"
            << "  [5] Add Teacher Comment\n"
            << "  [6] View Student Comments\n"
            << "  [0] Back\n";
        printSeparator('-');

        const int choice = readInt("  > ", 0, 6);
        if (choice == 0) break;
        switch (choice) {
            case 1: markAttendance();          break;
            case 2: editAttendance();          break;
            case 3: viewAttendanceByDate();    break;
            case 4: viewAttendanceByStudent(); break;
            case 5: addComment();              break;
            case 6: viewComments();            break;
        }
    }
}

void MenuController::handleReportMenu() {
    while (true) {
        printHeader("REPORTS & ANALYTICS");
        std::cout
            << "  [1] Attendance Analytics (on-screen)\n"
            << "  [2] Low Attendance Students (<75%)\n"
            << "  [3] Generate All Reports to Files\n"
            << "  [0] Back\n";
        printSeparator('-');

        const int choice = readInt("  > ", 0, 3);
        if (choice == 0) break;
        switch (choice) {
            case 1: showAnalytics();    break;
            case 2: showLowAttendance();break;
            case 3: saveReports();      break;
        }
    }
}

void MenuController::handleUndoRedo() {
    printHeader("UNDO / REDO");

    std::cout << "  Undo available : "
              << (attendance_.canUndo() ? attendance_.undoDescription() : "(none)") << "\n"
              << "  Redo available : "
              << (attendance_.canRedo() ? attendance_.redoDescription() : "(none)") << "\n\n"
              << "  [1] Undo\n"
              << "  [2] Redo\n"
              << "  [0] Back\n";
    printSeparator('-');

    const int choice = readInt("  > ", 0, 2);
    if (choice == 0) return;

    try {
        if (choice == 1) {
            attendance_.undo();
            std::cout << "\n  [OK] Undo successful.\n";
        } else {
            attendance_.redo();
            std::cout << "\n  [OK] Redo successful.\n";
        }
    } catch (const std::exception& e) {
        std::cout << "\n  [ERR] " << e.what() << "\n";
    }
    pressEnterToContinue();
}

// ──────────────────────────────────────────────────────────────────────────────
// Student Operations
// ──────────────────────────────────────────────────────────────────────────────

void MenuController::addStudent() {
    printHeader("ADD STUDENT");
    const std::string name       = readString("  Full Name       : ");
    const std::string section    = readString("  Section         : ");
    const std::string rollNumber = readString("  Roll Number     : ");
    const int         year       = readInt   ("  Enrollment Year : ", 1900, 2200);

    const int id = registry_.generateId();
    try {
        registry_.addStudent(Student{id, name, section, year, rollNumber});
        std::cout << "\n  [OK] Student added. Assigned ID: " << id << "\n";
        Logger::instance().info(
            "Student added: id=" + std::to_string(id) + " name=" + name);
    } catch (const std::exception& e) {
        std::cout << "\n  [ERR] " << e.what() << "\n";
        Logger::instance().error("addStudent failed: " + std::string(e.what()));
    }
    pressEnterToContinue();
}

void MenuController::removeStudent() {
    printHeader("REMOVE STUDENT");
    const int id = readInt("  Student ID: ", 1);

    const Student* s = registry_.findStudent(id);
    if (!s) {
        std::cout << "\n  [ERR] Student ID " << id << " not found.\n";
        pressEnterToContinue();
        return;
    }

    std::cout << "  Removing: [" << id << "] " << s->name()
              << " (" << s->rollNumber() << ")\n"
              << "  Are you sure? [y/N]: ";
    char confirm;
    std::cin >> confirm;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (confirm == 'y' || confirm == 'Y') {
        registry_.removeStudent(id);
        std::cout << "\n  [OK] Student removed.\n";
        Logger::instance().info("Student removed: id=" + std::to_string(id));
    } else {
        std::cout << "\n  Cancelled.\n";
    }
    pressEnterToContinue();
}

void MenuController::listStudents() {
    printHeader("ALL STUDENTS");

    if (registry_.count() == 0) {
        std::cout << "  No students registered.\n";
        pressEnterToContinue();
        return;
    }

    std::cout << std::left
              << std::setw(6)  << "ID"
              << std::setw(24) << "Name"
              << std::setw(10) << "Section"
              << std::setw(14) << "Roll No."
              << "Year\n";
    std::cout << std::string(60, '-') << "\n";

    for (auto it = registry_.all().begin(); it != registry_.all().end(); ++it) {
        const auto& s = it->second;
        std::cout << std::setw(6)  << it->first
                  << std::setw(24) << s.name().substr(0, 23)
                  << std::setw(10) << s.section()
                  << std::setw(14) << s.rollNumber()
                  << s.enrollmentYear() << "\n";
    }
    std::cout << "\n  Total: " << registry_.count() << " student(s).\n";
    pressEnterToContinue();
}

void MenuController::searchStudent() {
    printHeader("SEARCH STUDENT");
    const std::string query = readString("  Search name: ");
    const auto results = registry_.searchByName(query);

    if (results.empty()) {
        std::cout << "\n  No results for '" << query << "'.\n";
    } else {
        std::cout << "\n  Found " << results.size() << " result(s):\n";
        for (const auto* s : results) {
            std::cout << "    [" << s->id() << "] "
                      << s->name() << " | "
                      << s->section() << " | "
                      << s->rollNumber() << "\n";
        }
    }
    pressEnterToContinue();
}

// ──────────────────────────────────────────────────────────────────────────────
// Attendance Operations
// ──────────────────────────────────────────────────────────────────────────────

void MenuController::markAttendance() {
    printHeader("MARK ATTENDANCE");

    const Date date = readDate("  Date [YYYY-MM-DD, blank=today]: ");
    const int teacherId = readInt("  Your Teacher ID: ", 1);

    std::cout << "\n  Mark all students at once? [y/N]: ";
    char bulk;
    std::cin >> bulk;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (bulk == 'y' || bulk == 'Y') {
        if (registry_.count() == 0) {
            std::cout << "  No students registered.\n";
            pressEnterToContinue();
            return;
        }
        std::cout << "\n  Statuses: P=Present  A=Absent  L=Late  E=Excused  (blank=Present)\n";
        std::cout << std::string(52, '-') << "\n";

        int marked = 0, skipped = 0;
        for (auto it = registry_.all().begin(); it != registry_.all().end(); ++it) {
            const int id = it->first;
            const Student& student = it->second;
            if (attendance_.hasRecord(id, date)) {
                std::cout << "  [" << std::setw(4) << id << "] "
                          << std::left << std::setw(22) << student.name().substr(0, 21)
                          << "  (already marked, skipping)\n";
                ++skipped;
                continue;
            }
            std::cout << "  [" << std::setw(4) << id << "] "
                      << std::left << std::setw(22) << student.name().substr(0, 21)
                      << "> ";
            std::string input;
            std::getline(std::cin, input);

            AttendanceStatus status = AttendanceStatus::PRESENT;
            if (!input.empty()) {
                const char c = static_cast<char>(std::toupper(
                    static_cast<unsigned char>(input[0])));
                if      (c == 'A') status = AttendanceStatus::ABSENT;
                else if (c == 'L') status = AttendanceStatus::LATE;
                else if (c == 'E') status = AttendanceStatus::EXCUSED;
            }

            try {
                attendance_.markAttendance(id, date, status, teacherId);
                ++marked;
            } catch (const std::exception& e) {
                std::cout << "    [ERR] " << e.what() << "\n";
                ++skipped;
            }
        }
        std::cout << "\n  [OK] Marked: " << marked
                  << "  |  Skipped: " << skipped << "\n";

    } else {
        const int studentId = readInt("  Student ID: ", 1);
        if (!registry_.exists(studentId)) {
            std::cout << "\n  [ERR] Student ID " << studentId << " not found.\n";
            pressEnterToContinue();
            return;
        }
        const AttendanceStatus status = readStatus("  Status [PRESENT/ABSENT/LATE/EXCUSED]: ");
        try {
            attendance_.markAttendance(studentId, date, status, teacherId);
            std::cout << "\n  [OK] Attendance marked.\n";
        } catch (const std::exception& e) {
            std::cout << "\n  [ERR] " << e.what() << "\n";
        }
    }
    pressEnterToContinue();
}

void MenuController::editAttendance() {
    printHeader("EDIT ATTENDANCE");

    const int studentId = readInt("  Student ID: ", 1);
    if (!registry_.exists(studentId)) {
        std::cout << "\n  [ERR] Student not found.\n";
        pressEnterToContinue();
        return;
    }

    const Date date = readDate("  Date [YYYY-MM-DD]: ");
    if (!attendance_.hasRecord(studentId, date)) {
        std::cout << "\n  [ERR] No record for student #" << studentId
                  << " on " << date.toString() << ".\n";
        pressEnterToContinue();
        return;
    }

    const int teacherId = readInt("  Your Teacher ID: ", 1);
    const AttendanceStatus status = readStatus("  New Status [PRESENT/ABSENT/LATE/EXCUSED]: ");

    try {
        attendance_.editAttendance(studentId, date, status, teacherId);
        std::cout << "\n  [OK] Attendance updated.\n";
    } catch (const std::exception& e) {
        std::cout << "\n  [ERR] " << e.what() << "\n";
    }
    pressEnterToContinue();
}

void MenuController::viewAttendanceByDate() {
    printHeader("VIEW ATTENDANCE BY DATE");
    const Date date = readDate("  Date [YYYY-MM-DD]: ");

    const auto& records = attendance_.recordsForDate(date);
    if (records.empty()) {
        std::cout << "\n  No records for " << date.toString() << ".\n";
        pressEnterToContinue();
        return;
    }

    std::cout << "\n  Date: " << date.toString()
              << "  (" << records.size() << " record(s))\n";
    std::cout << std::string(52, '-') << "\n";
    std::cout << std::left
              << std::setw(6)  << "ID"
              << std::setw(24) << "Name"
              << "Status\n";
    std::cout << std::string(52, '-') << "\n";

    for (const auto& r : records) {
        std::string name = "Unknown";
        if (const Student* s = registry_.findStudent(r.studentId)) name = s->name();
        std::cout << std::setw(6)  << r.studentId
                  << std::setw(24) << name.substr(0, 23)
                  << statusToString(r.status) << "\n";
    }
    pressEnterToContinue();
}

void MenuController::viewAttendanceByStudent() {
    printHeader("VIEW ATTENDANCE BY STUDENT");
    const int studentId = readInt("  Student ID: ", 1);

    const Student* s = registry_.findStudent(studentId);
    if (!s) {
        std::cout << "\n  [ERR] Student not found.\n";
        pressEnterToContinue();
        return;
    }

    const auto records = attendance_.recordsForStudent(studentId);
    std::cout << "\n  Student: [" << studentId << "] " << s->name()
              << "  (" << records.size() << " record(s))\n";
    std::cout << std::string(48, '-') << "\n";

    for (const auto& r : records) {
        std::cout << "  " << r.date.toString()
                  << "  " << std::left << std::setw(10) << statusToString(r.status)
                  << "  Teacher #" << r.teacherId
                  << "  [" << r.timestamp << "]\n";
    }

    if (records.empty()) std::cout << "  No attendance records.\n";
    pressEnterToContinue();
}

void MenuController::addComment() {
    printHeader("ADD TEACHER COMMENT");

    const int studentId = readInt("  Student ID: ", 1);
    if (!registry_.exists(studentId)) {
        std::cout << "\n  [ERR] Student not found.\n";
        pressEnterToContinue();
        return;
    }

    const Date date      = readDate("  Date [YYYY-MM-DD, blank=today]: ");
    const int  teacherId = readInt ("  Your Teacher ID: ", 1);
    const std::string text = readString("  Comment: ");

    if (text.empty()) {
        std::cout << "\n  [ERR] Comment text cannot be empty.\n";
        pressEnterToContinue();
        return;
    }

    Comment c(studentId, date, teacherId, text, ams::util::currentTimestamp());
    attendance_.addComment(std::move(c));

    std::cout << "\n  [OK] Comment saved.\n";
    Logger::instance().info(
        "Comment added: student=" + std::to_string(studentId) +
        " date=" + date.toString() +
        " teacher=" + std::to_string(teacherId));
    pressEnterToContinue();
}

void MenuController::viewComments() {
    printHeader("VIEW STUDENT COMMENTS");

    const int studentId = readInt("  Student ID: ", 1);
    const Student* s = registry_.findStudent(studentId);
    if (!s) {
        std::cout << "\n  [ERR] Student not found.\n";
        pressEnterToContinue();
        return;
    }

    const auto comments = attendance_.allCommentsForStudent(studentId);
    std::cout << "\n  Comments for [" << studentId << "] " << s->name()
              << " (" << comments.size() << " total)\n";
    std::cout << std::string(50, '-') << "\n";

    if (comments.empty()) {
        std::cout << "  No comments on record.\n";
    } else {
        for (const auto& c : comments) {
            std::cout << "  Date     : " << c.date.toString()  << "\n"
                      << "  Teacher  : #" << c.authorId        << "\n"
                      << "  Time     : " << c.timestamp        << "\n"
                      << "  Remark   : " << c.text             << "\n"
                      << std::string(30, '.') << "\n";
        }
    }
    pressEnterToContinue();
}

// ──────────────────────────────────────────────────────────────────────────────
// Reports
// ──────────────────────────────────────────────────────────────────────────────

void MenuController::showAnalytics() {
    printHeader("ATTENDANCE ANALYTICS");

    if (registry_.count() == 0) {
        std::cout << "  No students registered.\n";
        pressEnterToContinue();
        return;
    }

    const auto analytics = AnalyticsEngine::computeAll(registry_, attendance_);

    std::cout << std::left
              << std::setw(6)  << "ID"
              << std::setw(20) << "Name"
              << std::setw(7)  << "Total"
              << std::setw(7)  << "Pres."
              << std::setw(7)  << "Abs."
              << std::setw(9)  << "Att. %"
              << "Flag\n";
    std::cout << std::string(58, '-') << "\n";

    for (const auto& a : analytics) {
        std::cout << std::setw(6)  << a.studentId
                  << std::setw(20) << a.studentName.substr(0, 19)
                  << std::setw(7)  << a.totalDays
                  << std::setw(7)  << a.presentDays
                  << std::setw(7)  << a.absentDays
                  << std::setw(9)  << std::fixed << std::setprecision(1)
                                   << a.attendancePercent
                  << (a.lowAttendance ? "  *** LOW ***" : "") << "\n";
    }

    const auto lowCount = static_cast<int>(
        std::count_if(analytics.begin(), analytics.end(),
                      [](const StudentAnalytics& a) { return a.lowAttendance; }));
    std::cout << "\n  Low-attendance students: " << lowCount
              << " of " << analytics.size() << "\n";
    pressEnterToContinue();
}

void MenuController::showLowAttendance() {
    printHeader("LOW ATTENDANCE  (<75%)");

    const auto analytics   = AnalyticsEngine::computeAll(registry_, attendance_);
    const auto lowStudents = AnalyticsEngine::getLowAttendance(analytics);

    if (lowStudents.empty()) {
        std::cout << "  All students meet the 75% attendance requirement.\n";
    } else {
        std::cout << "  " << lowStudents.size()
                  << " student(s) below threshold:\n\n";
        for (const auto& a : lowStudents) {
            std::cout << "  [" << a.studentId << "] " << a.studentName << "\n"
                      << "       Attendance: " << std::fixed << std::setprecision(1)
                      << a.attendancePercent << "%"
                      << "  (absent " << a.absentDays << "/" << a.totalDays << " days)\n";
        }
    }
    pressEnterToContinue();
}

void MenuController::saveReports() {
    printHeader("GENERATE ALL REPORTS");

    try {
        const auto analytics = AnalyticsEngine::computeAll(registry_, attendance_);
        reportGen_.generateProjectSummary();
        reportGen_.generateAnalyticsReport(analytics, registry_);
        reportGen_.generateLowAttendanceReport(analytics);
        std::cout << "\n  [OK] Reports saved to: " << reportGen_.reportsDir() << "\n";
        Logger::instance().info("All reports generated");
    } catch (const std::exception& e) {
        std::cout << "\n  [ERR] " << e.what() << "\n";
        Logger::instance().error("Report generation failed: " + std::string(e.what()));
    }
    pressEnterToContinue();
}

// ──────────────────────────────────────────────────────────────────────────────
// Input Helpers
// ──────────────────────────────────────────────────────────────────────────────

int MenuController::readInt(const std::string& prompt, int minVal, int maxVal) {
    while (true) {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);

        if (line.empty()) {
            // Accept empty as 0 only if 0 is within range
            if (minVal <= 0 && maxVal >= 0) return 0;
            std::cout << "  Please enter a number.\n";
            continue;
        }

        try {
            const int val = std::stoi(line);
            if (val >= minVal && val <= maxVal) return val;
            std::cout << "  Enter a number between " << minVal << " and " << maxVal << ".\n";
        } catch (...) {
            std::cout << "  Invalid input — please enter a number.\n";
        }
    }
}

std::string MenuController::readString(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    // Trim trailing whitespace
    while (!line.empty() && std::isspace(static_cast<unsigned char>(line.back())))
        line.pop_back();
    return line;
}

Date MenuController::readDate(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);

        if (line.empty()) return Date::today();

        try {
            return Date{line};
        } catch (const std::exception& e) {
            std::cout << "  Invalid date: " << e.what() << "\n";
        }
    }
}

AttendanceStatus MenuController::readStatus(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);

        // Convert to uppercase for comparison
        std::transform(line.begin(), line.end(), line.begin(),
                       [](unsigned char c) { return std::toupper(c); });

        // Accept full word or single-character shortcut
        if (line == "PRESENT" || line == "P") return AttendanceStatus::PRESENT;
        if (line == "ABSENT"  || line == "A") return AttendanceStatus::ABSENT;
        if (line == "LATE"    || line == "L") return AttendanceStatus::LATE;
        if (line == "EXCUSED" || line == "E") return AttendanceStatus::EXCUSED;

        std::cout << "  Enter PRESENT/ABSENT/LATE/EXCUSED (or P/A/L/E).\n";
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// Display Helpers
// ──────────────────────────────────────────────────────────────────────────────

void MenuController::printSeparator(char c, int width) {
    std::cout << std::string(static_cast<size_t>(width), c) << "\n";
}

void MenuController::printHeader(const std::string& title) {
    std::cout << "\n";
    printSeparator('=', 60);
    std::cout << "  " << title << "\n";
    printSeparator('=', 60);
}

void MenuController::pressEnterToContinue() {
    std::cout << "\n  Press Enter to continue...";
    std::string dummy;
    std::getline(std::cin, dummy);
}

} // namespace ams
