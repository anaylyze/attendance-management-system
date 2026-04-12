/**
 * tests.cpp — Self-contained test suite for the Attendance Management System.
 *
 * No external framework required (no Google Test, no Catch2).
 * Uses a minimal macro-based assertion harness written inline.
 *
 * Test coverage:
 *   - Date:             construction, parsing, comparison, hash collision check
 *   - StudentRegistry:  CRUD, ID generation, name search, duplicate guard
 *   - AttendanceManager: mark, duplicate guard, edit, undo, redo, comment API
 *   - AnalyticsEngine:  per-student compute, low-attendance filter, edge cases
 *   - FileManager:      save/load round-trip for students, attendance, comments
 *   - UndoRedoStack:    execute, undo, redo, clear, error paths
 *
 * Build:
 *   g++ -std=c++14 -Iinclude -o tests.exe test/tests.cpp (src files)
 *   .\tests.exe
 */

#include "entity/Date.h"
#include "entity/Student.h"
#include "entity/Teacher.h"
#include "entity/AttendanceRecord.h"
#include "entity/Comment.h"
#include "manager/StudentRegistry.h"
#include "manager/AttendanceManager.h"
#include "service/AnalyticsEngine.h"
#include "service/FileManager.h"
#include "service/Logger.h"
#include "service/UndoRedoStack.h"

#include <iostream>
#include <sstream>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <string>
#include <sys/stat.h>

#ifdef _WIN32
#  include <direct.h>
#  define AMS_MKDIR(p) _mkdir(p)
#  define AMS_RMDIR(p) _rmdir(p)
#else
#  include <sys/types.h>
#  define AMS_MKDIR(p) mkdir((p), 0755)
#  define AMS_RMDIR(p) rmdir(p)
#endif

// ─────────────────────────────────────────────────────────────────────────────
// Micro Test Harness
// ─────────────────────────────────────────────────────────────────────────────

struct TestResults {
    int passed{0};
    int failed{0};
    std::vector<std::string> failures;
};

static TestResults g_results;
static std::string g_currentSuite;

#define SUITE(name)  do { g_currentSuite = (name); \
    std::cout << "\n[SUITE] " << g_currentSuite << "\n" \
              << std::string(50, '-') << "\n"; } while(0)

#define TEST(desc, expr) do {                                          \
    try {                                                              \
        if (!(expr)) {                                                 \
            ++g_results.failed;                                        \
            std::string msg = "  FAIL  " + std::string(desc);         \
            g_results.failures.push_back("[" + g_currentSuite + "] " + msg); \
            std::cout << msg << "\n";                                  \
        } else {                                                       \
            ++g_results.passed;                                        \
            std::cout << "  PASS  " << (desc) << "\n";                \
        }                                                              \
    } catch (const std::exception& e) {                               \
        ++g_results.failed;                                            \
        std::string msg = "  EXCP  " + std::string(desc) + " -> " + e.what(); \
        g_results.failures.push_back("[" + g_currentSuite + "] " + msg); \
        std::cout << msg << "\n";                                      \
    }                                                                  \
} while(0)

#define TEST_THROWS(desc, expr) do {                                   \
    bool threw = false;                                                \
    try { expr; }                                                      \
    catch (...) { threw = true; }                                      \
    if (threw) {                                                       \
        ++g_results.passed;                                            \
        std::cout << "  PASS  " << (desc) << "  [threw as expected]\n"; \
    } else {                                                           \
        ++g_results.failed;                                            \
        std::string msg = "  FAIL  " + std::string(desc) + "  [expected throw, none raised]"; \
        g_results.failures.push_back("[" + g_currentSuite + "] " + msg); \
        std::cout << msg << "\n";                                      \
    }                                                                  \
} while(0)

// ─────────────────────────────────────────────────────────────────────────────
// Test: Date
// ─────────────────────────────────────────────────────────────────────────────

void testDate() {
    SUITE("Date");

    // Basic construction
    ams::Date d1{2024, 3, 15};
    TEST("year()",  d1.year()  == 2024);
    TEST("month()", d1.month() == 3);
    TEST("day()",   d1.day()   == 15);
    TEST("toString()", d1.toString() == "2024-03-15");
    TEST("isValid()", d1.isValid());

    // String parsing
    ams::Date d2{"2024-03-15"};
    TEST("Parse YYYY-MM-DD", d2 == d1);

    // Comparators
    ams::Date earlier{2024, 1, 1};
    ams::Date later{2024, 12, 31};
    TEST("operator<",  earlier < later);
    TEST("operator>",  later   > earlier);
    TEST("operator<=", earlier <= d1);
    TEST("operator>=", later   >= d1);
    TEST("operator==", d1 == d2);
    TEST("operator!=", d1 != earlier);

    // Leap year handling
    ams::Date leapDay{2024, 2, 29};
    TEST("Leap day valid", leapDay.isValid());

    // Edge: invalid dates throw
    TEST_THROWS("Invalid month 13",    ams::Date(2024, 13, 1));
    TEST_THROWS("Invalid day 0",       ams::Date(2024, 1, 0));
    TEST_THROWS("Feb 29 non-leap",     ams::Date(2023, 2, 29));
    TEST_THROWS("Bad parse format",    ams::Date("2024/03/15"));
    TEST_THROWS("Bad parse non-digit", ams::Date("YYYY-MM-DD"));

    // today() returns valid date
    ams::Date today = ams::Date::today();
    TEST("today() is valid", today.isValid());
    TEST("today() year >= 2024", today.year() >= 2024);

    // CSV round-trip
    ams::Date d3{"2000-01-01"};
    TEST("CSV round-trip", ams::Date{d3.toString()} == d3);

    // Hash: two equal dates must hash equally
    std::hash<ams::Date> hasher;
    TEST("Equal dates same hash", hasher(d1) == hasher(d2));
    TEST("Different dates different hash", hasher(earlier) != hasher(later));

    // Default-constructed is invalid
    ams::Date def{};
    TEST("Default-constructed is invalid", !def.isValid());
}

// ─────────────────────────────────────────────────────────────────────────────
// Test: Student & StudentRegistry
// ─────────────────────────────────────────────────────────────────────────────

void testStudentRegistry() {
    SUITE("StudentRegistry");

    ams::StudentRegistry reg;
    TEST("Empty registry count is 0", reg.count() == 0);

    // Add students
    reg.addStudent(ams::Student{1, "Alice Johnson", "CS-A", 2022, "CS001"});
    reg.addStudent(ams::Student{2, "Bob Smith",     "CS-B", 2021, "CS002"});
    reg.addStudent(ams::Student{3, "Carol White",   "CS-A", 2022, "CS003"});
    TEST("Count after 3 adds", reg.count() == 3);

    // exists()
    TEST("exists(1) true",  reg.exists(1));
    TEST("exists(99) false", !reg.exists(99));

    // getStudent
    TEST("getStudent name", reg.getStudent(1).name() == "Alice Johnson");
    TEST_THROWS("getStudent invalid ID", reg.getStudent(999));

    // findStudent
    TEST("findStudent found",     reg.findStudent(2) != nullptr);
    TEST("findStudent not found", reg.findStudent(99) == nullptr);

    // generateId
    TEST("generateId = max+1", reg.generateId() == 4);

    // Duplicate guard
    TEST_THROWS("Duplicate ID throws",
        reg.addStudent(ams::Student{1, "Duplicate", "X", 2020, "X001"}));

    // searchByName (case-insensitive)
    auto r1 = reg.searchByName("alice");
    TEST("Search 'alice' finds 1", r1.size() == 1);
    TEST("Search result is Alice", r1[0]->name() == "Alice Johnson");

    auto r2 = reg.searchByName("smith");
    TEST("Search 'smith' finds Bob", r2.size() == 1 && r2[0]->id() == 2);

    auto r3 = reg.searchByName("xyz_not_exist");
    TEST("Search no match returns empty", r3.empty());

    // bySection
    auto sec = reg.bySection("CS-A");
    TEST("bySection CS-A returns 2 students", sec.size() == 2);

    // removeStudent
    TEST("removeStudent existing", reg.removeStudent(2));
    TEST("removeStudent nonexistent returns false", !reg.removeStudent(999));
    TEST("Count after remove", reg.count() == 2);
    TEST("Removed student gone", !reg.exists(2));

    // generateId after removal (max key + 1, not fill gaps)
    TEST("generateId after remove skips gap", reg.generateId() == 4);

    // CSV round-trip
    ams::Student s{42, "Test Name", "SEC-Z", 2023, "TZ042"};
    std::string row = s.toCSVRow();
    ams::Student s2 = ams::Student::fromCSVRow(row);
    TEST("Student CSV round-trip id",          s2.id()             == 42);
    TEST("Student CSV round-trip name",        s2.name()           == "Test Name");
    TEST("Student CSV round-trip section",     s2.section()        == "SEC-Z");
    TEST("Student CSV round-trip year",        s2.enrollmentYear() == 2023);
    TEST("Student CSV round-trip rollNumber",  s2.rollNumber()     == "TZ042");

    // Invalid construction
    TEST_THROWS("Empty name throws",    ams::Student(1, "",    "A", 2020, "R1"));
    TEST_THROWS("Empty section throws", ams::Student(1, "X",  "",  2020, "R1"));
    TEST_THROWS("Zero ID throws",       ams::Student(0, "X",  "A", 2020, "R1"));
    TEST_THROWS("Negative ID throws",   ams::Student(-1,"X",  "A", 2020, "R1"));
}

// ─────────────────────────────────────────────────────────────────────────────
// Test: AttendanceManager
// ─────────────────────────────────────────────────────────────────────────────

void testAttendanceManager() {
    SUITE("AttendanceManager");
    using Status = ams::AttendanceStatus;

    // Suppress logger noise in tests
    ams::Logger::instance().init("data/test_run.log");

    ams::AttendanceManager mgr;
    ams::Date d1{2024, 6, 10};
    ams::Date d2{2024, 6, 11};

    // Basic mark
    mgr.markAttendance(1, d1, Status::PRESENT, 99);
    TEST("hasRecord after mark", mgr.hasRecord(1, d1));
    TEST("hasRecord wrong date", !mgr.hasRecord(1, d2));
    TEST("hasRecord wrong student", !mgr.hasRecord(2, d1));

    // Duplicate guard
    TEST_THROWS("Duplicate mark throws",
        mgr.markAttendance(1, d1, Status::ABSENT, 99));

    // Multiple students same date
    mgr.markAttendance(2, d1, Status::ABSENT,  99);
    mgr.markAttendance(3, d1, Status::LATE,    99);
    const auto& dayRecs = mgr.recordsForDate(d1);
    TEST("recordsForDate count", dayRecs.size() == 3);

    // recordsForDate empty date
    ams::Date empty{2024, 1, 1};
    TEST("recordsForDate empty", mgr.recordsForDate(empty).empty());

    // recordsForStudent (sorted by date)
    mgr.markAttendance(1, d2, Status::LATE, 99);
    auto recs = mgr.recordsForStudent(1);
    TEST("recordsForStudent count", recs.size() == 2);
    TEST("recordsForStudent sorted", recs[0].date < recs[1].date);
    TEST("recordsForStudent first status", recs[0].status == Status::PRESENT);

    // Edit attendance
    mgr.editAttendance(1, d1, Status::EXCUSED, 99);
    // After edit, status should have changed
    const auto& updated = mgr.recordsForDate(d1);
    bool found = false;
    for (const auto& r : updated)
        if (r.studentId == 1) { found = (r.status == Status::EXCUSED); break; }
    TEST("editAttendance updates status", found);

    // Edit non-existent throws
    TEST_THROWS("Edit no record throws",
        mgr.editAttendance(1, ams::Date{2025,1,1}, Status::ABSENT, 99));

    // Undo: revert edit
    mgr.undo();
    const auto& reverted = mgr.recordsForDate(d1);
    bool revFound = false;
    for (const auto& r : reverted)
        if (r.studentId == 1) { revFound = (r.status == Status::PRESENT); break; }
    TEST("undo reverts edit", revFound);

    // Redo: re-apply edit
    mgr.redo();
    const auto& redone = mgr.recordsForDate(d1);
    bool redoFound = false;
    for (const auto& r : redone)
        if (r.studentId == 1) { redoFound = (r.status == Status::EXCUSED); break; }
    TEST("redo re-applies edit", redoFound);

    // Undo mark: removes the record
    // Mark a new record, then undo it
    ams::Date d3{2024, 7, 1};
    mgr.markAttendance(1, d3, Status::ABSENT, 99);
    TEST("Record exists before undo-mark", mgr.hasRecord(1, d3));
    mgr.undo();
    TEST("Record gone after undo-mark", !mgr.hasRecord(1, d3));

    // canUndo / canRedo
    TEST("canRedo after undo-mark", mgr.canRedo());

    // Comments
    ams::Comment c{1, d1, 99, "Excellent work today", "2024-06-10T10:00:00"};
    mgr.addComment(c);
    auto comments = mgr.commentsFor(1, d1);
    TEST("Comment count for (1, d1)", comments.size() == 1);
    TEST("Comment text", comments[0].text == "Excellent work today");

    auto allComments = mgr.allCommentsForStudent(1);
    TEST("allCommentsForStudent", !allComments.empty());

    // Comment CSV round-trip
    std::string cRow = c.toCSVRow();
    ams::Comment c2 = ams::Comment::fromCSVRow(cRow);
    TEST("Comment CSV round-trip studentId",  c2.studentId == 1);
    TEST("Comment CSV round-trip authorId",   c2.authorId  == 99);
    TEST("Comment CSV round-trip text",       c2.text      == "Excellent work today");
    TEST("Comment CSV round-trip date",       c2.date      == d1);

    // clear()
    mgr.clear();
    TEST("clear: no records", mgr.allRecords().empty());
    TEST("clear: no undo",    !mgr.canUndo());
}

// ─────────────────────────────────────────────────────────────────────────────
// Test: UndoRedoStack isolation
// ─────────────────────────────────────────────────────────────────────────────

void testUndoRedoStack() {
    SUITE("UndoRedoStack");
    using Status = ams::AttendanceStatus;

    ams::AttendanceManager mgr;
    ams::Date d{2024, 9, 1};

    TEST("canUndo empty", !mgr.canUndo());
    TEST("canRedo empty", !mgr.canRedo());

    TEST_THROWS("Undo on empty throws",  mgr.undo());
    TEST_THROWS("Redo on empty throws",  mgr.redo());

    // Execute 3 commands
    mgr.markAttendance(1, d,                    Status::PRESENT, 1);
    mgr.markAttendance(2, d,                    Status::ABSENT,  1);
    mgr.markAttendance(3, d,                    Status::LATE,    1);
    TEST("canUndo after 3 marks", mgr.canUndo());
    TEST("canRedo after 3 marks (none yet)", !mgr.canRedo());

    // Undo twice
    mgr.undo(); // removes student 3
    mgr.undo(); // removes student 2
    TEST("After 2 undos, student 3 gone", !mgr.hasRecord(3, d));
    TEST("After 2 undos, student 2 gone", !mgr.hasRecord(2, d));
    TEST("After 2 undos, student 1 remains", mgr.hasRecord(1, d));
    TEST("canRedo after 2 undos", mgr.canRedo());

    // New command clears redo stack
    mgr.markAttendance(4, d, Status::EXCUSED, 1);
    TEST("canRedo cleared after new command", !mgr.canRedo());
    TEST("New mark recorded", mgr.hasRecord(4, d));
}

// ─────────────────────────────────────────────────────────────────────────────
// Test: AnalyticsEngine
// ─────────────────────────────────────────────────────────────────────────────

void testAnalyticsEngine() {
    SUITE("AnalyticsEngine");
    using Status = ams::AttendanceStatus;

    ams::StudentRegistry reg;
    ams::AttendanceManager mgr;

    reg.addStudent(ams::Student{1, "Good Student",   "A", 2022, "G001"});
    reg.addStudent(ams::Student{2, "Poor Student",   "A", 2022, "P001"});
    reg.addStudent(ams::Student{3, "New Student",    "B", 2022, "N001"}); // no records

    // Good student: 4 present out of 4 = 100%
    mgr.markAttendance(1, ams::Date{2024,1,1}, Status::PRESENT, 9);
    mgr.markAttendance(1, ams::Date{2024,1,2}, Status::PRESENT, 9);
    mgr.markAttendance(1, ams::Date{2024,1,3}, Status::LATE,    9); // counts
    mgr.markAttendance(1, ams::Date{2024,1,4}, Status::EXCUSED, 9); // counts

    // Poor student: 1 present out of 4 = 25%
    mgr.markAttendance(2, ams::Date{2024,1,1}, Status::ABSENT, 9);
    mgr.markAttendance(2, ams::Date{2024,1,2}, Status::ABSENT, 9);
    mgr.markAttendance(2, ams::Date{2024,1,3}, Status::ABSENT, 9);
    mgr.markAttendance(2, ams::Date{2024,1,4}, Status::PRESENT,9);

    auto analytics = ams::AnalyticsEngine::computeAll(reg, mgr);
    TEST("computeAll returns 3 entries",    analytics.size() == 3);

    // Sorted ascending (worst first): poor (25%) < new (100% default) ... 
    // Actually: poor=25%, good=100%, new=100% (no records → 100%)
    // So order: poor (25%), then good/new at 100%
    TEST("Worst-first sort: poor student first", analytics[0].studentId == 2);
    TEST("Poor student 25%", analytics[0].attendancePercent < 30.0);
    TEST("Poor student flagged low", analytics[0].lowAttendance);

    // Good student: 100%
    bool goodFound = false;
    for (const auto& a : analytics) {
        if (a.studentId == 1) {
            goodFound = (a.attendancePercent == 100.0 && !a.lowAttendance);
        }
    }
    TEST("Good student 100%, not flagged", goodFound);

    // New student: 0 records → 100%
    bool newFound = false;
    for (const auto& a : analytics) {
        if (a.studentId == 3) {
            newFound = (a.attendancePercent == 100.0 && !a.lowAttendance);
        }
    }
    TEST("New student (0 records) = 100%, not flagged", newFound);

    // getLowAttendance
    auto low = ams::AnalyticsEngine::getLowAttendance(analytics);
    TEST("getLowAttendance count == 1", low.size() == 1);
    TEST("getLowAttendance is poor student", low[0].studentId == 2);

    // Individual per-student
    auto recs1 = mgr.recordsForStudent(1);
    auto a1    = ams::AnalyticsEngine::computeForStudent(reg.getStudent(1), recs1);
    TEST("Per-student: present count",  a1.presentDays == 2);
    TEST("Per-student: late count",     a1.lateDays    == 1);
    TEST("Per-student: excused count",  a1.excusedDays == 1);
    TEST("Per-student: absent count",   a1.absentDays  == 0);
    TEST("Per-student: total days",     a1.totalDays   == 4);
    TEST("Per-student: 100%",           a1.attendancePercent == 100.0);

    // Custom threshold test
    auto a2recs = mgr.recordsForStudent(2);
    auto a2_50  = ams::AnalyticsEngine::computeForStudent(reg.getStudent(2), a2recs, 50.0);
    TEST("Custom threshold 50%: poor student (25%) still low", a2_50.lowAttendance);
    auto a2_20  = ams::AnalyticsEngine::computeForStudent(reg.getStudent(2), a2recs, 20.0);
    TEST("Custom threshold 20%: poor student (25%) passes", !a2_20.lowAttendance);
}

// ─────────────────────────────────────────────────────────────────────────────
// Test: FileManager round-trip
// ─────────────────────────────────────────────────────────────────────────────

void testFileManager() {
    SUITE("FileManager (CSV round-trip)");
    using Status = ams::AttendanceStatus;

    const std::string testDir = "data/test_tmp";
    AMS_MKDIR("data");
    AMS_MKDIR(testDir.c_str());

    // Build original state
    ams::StudentRegistry   reg;
    ams::AttendanceManager mgr;

    reg.addStudent(ams::Student{1, "Alpha",  "X", 2022, "X001"});
    reg.addStudent(ams::Student{2, "Beta",   "Y", 2021, "Y002"});
    reg.addStudent(ams::Student{3, "Gamma",  "X", 2023, "X003"});

    ams::Date d1{2024, 5, 1};
    ams::Date d2{2024, 5, 2};
    mgr.markAttendance(1, d1, Status::PRESENT, 10);
    mgr.markAttendance(2, d1, Status::ABSENT,  10);
    mgr.markAttendance(3, d1, Status::LATE,    10);
    mgr.markAttendance(1, d2, Status::EXCUSED, 10);

    mgr.addComment(ams::Comment{1, d1, 10, "Good focus", "2024-05-01T09:00:00"});
    mgr.addComment(ams::Comment{2, d1, 10, "Was absent; parent called", "2024-05-01T10:00:00"});

    // Save
    ams::FileManager fm{testDir};
    fm.saveAll(reg, mgr);

    // Load into fresh objects
    ams::StudentRegistry   reg2;
    ams::AttendanceManager mgr2;
    fm.loadAll(reg2, mgr2);

    // Verify students
    TEST("Loaded student count", reg2.count() == 3);
    TEST("Student 1 name", reg2.getStudent(1).name()    == "Alpha");
    TEST("Student 2 section", reg2.getStudent(2).section() == "Y");
    TEST("Student 3 rollNumber", reg2.getStudent(3).rollNumber() == "X003");

    // Verify attendance
    TEST("Attendance: student 1 d1", mgr2.hasRecord(1, d1));
    TEST("Attendance: student 2 d1", mgr2.hasRecord(2, d1));
    TEST("Attendance: student 1 d2", mgr2.hasRecord(1, d2));
    TEST("Attendance: student 3 d1", mgr2.hasRecord(3, d1));
    TEST("Attendance total day1 records",
         mgr2.recordsForDate(d1).size() == 3);

    // Verify status preserved
    const auto& d1recs = mgr2.recordsForDate(d1);
    Status s1 = Status::ABSENT;
    for (const auto& r : d1recs) if (r.studentId == 1) s1 = r.status;
    TEST("Status round-trip: PRESENT", s1 == Status::PRESENT);

    // Verify comments
    auto c1 = mgr2.commentsFor(1, d1);
    TEST("Comment round-trip count", c1.size() == 1);
    TEST("Comment round-trip text",  c1[0].text == "Good focus");
    TEST("Comment round-trip author", c1[0].authorId == 10);

    // Cleanup test directory files
    remove((testDir + "/students.csv").c_str());
    remove((testDir + "/attendance.csv").c_str());
    remove((testDir + "/comments.csv").c_str());
    AMS_RMDIR(testDir.c_str());
    struct stat sb;
    TEST("Test dir cleaned up", stat(testDir.c_str(), &sb) != 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Test: AttendanceRecord CSV
// ─────────────────────────────────────────────────────────────────────────────

void testAttendanceRecordCSV() {
    SUITE("AttendanceRecord CSV");
    using Status = ams::AttendanceStatus;

    for (auto status : {Status::PRESENT, Status::ABSENT, Status::LATE, Status::EXCUSED}) {
        ams::AttendanceRecord r(42, ams::Date{2024,8,20}, status, 7, "2024-08-20T08:00:00");
        std::string row = r.toCSVRow();
        ams::AttendanceRecord r2 = ams::AttendanceRecord::fromCSVRow(row);
        bool ok = (r2.studentId == 42) &&
                  (r2.date == ams::Date{2024,8,20}) &&
                  (r2.status == status) &&
                  (r2.teacherId == 7);
        TEST("CSV round-trip: " + ams::statusToString(status), ok);
    }

    // Malformed row throws
    TEST_THROWS("Malformed row (too few fields)",
        ams::AttendanceRecord::fromCSVRow("1,2024-01-01,PRESENT"));
    TEST_THROWS("Unknown status string",
        ams::AttendanceRecord::fromCSVRow("1,2024-01-01,UNKNOWN,5,ts"));
}

// ─────────────────────────────────────────────────────────────────────────────
// Main
// ─────────────────────────────────────────────────────────────────────────────

int main() {
    AMS_MKDIR("data");

    // Silence logger in tests (redirect to test log)
    ams::Logger::instance().init("data/test_run.log");

    std::cout << "\n"
              << "============================================================\n"
              << "  ATTENDANCE MANAGEMENT SYSTEM — TEST SUITE\n"
              << "============================================================\n";

    testDate();
    testStudentRegistry();
    testAttendanceManager();
    testUndoRedoStack();
    testAnalyticsEngine();
    testFileManager();
    testAttendanceRecordCSV();

    // ── Summary ──────────────────────────────────────────────────────────────
    std::cout << "\n"
              << "============================================================\n"
              << "  RESULTS\n"
              << "============================================================\n"
              << "  Passed : " << g_results.passed << "\n"
              << "  Failed : " << g_results.failed << "\n";

    if (!g_results.failures.empty()) {
        std::cout << "\n  FAILURES:\n";
        for (const auto& f : g_results.failures) {
            std::cout << "  " << f << "\n";
        }
    }

    const bool allPassed = (g_results.failed == 0);
    std::cout << "\n  " << (allPassed ? "[ALL TESTS PASSED]" : "[SOME TESTS FAILED]")
              << "\n============================================================\n\n";

    return allPassed ? 0 : 1;
}
