#pragma once

#include "manager/StudentRegistry.h"
#include "manager/AttendanceManager.h"
#include "service/FileManager.h"
#include "service/ReportGenerator.h"
#include "entity/AttendanceStatus.h"
#include "entity/Date.h"
#include <string>
#include <limits>

namespace ams {

/**
 * Terminal UI controller — the only class that performs console I/O.
 * Contains NO business logic; all operations delegate to the manager/service layer.
 *
 * Lifecycle:
 *   run() → blocking event loop → returns when user selects Exit.
 *   On exit, data is saved automatically.
 */
class MenuController {
public:
    MenuController(StudentRegistry&   registry,
                   AttendanceManager& attendance,
                   FileManager&       fileManager,
                   ReportGenerator&   reportGen);

    void run(); ///< Blocking event loop entry point

private:
    StudentRegistry&   registry_;
    AttendanceManager& attendance_;
    FileManager&       fileManager_;
    ReportGenerator&   reportGen_;
    bool               running_{true};

    // === Top-level menus ===
    void showMainMenu();
    void handleStudentMenu();
    void handleAttendanceMenu();
    void handleReportMenu();
    void handleUndoRedo();

    // === Student operations ===
    void addStudent();
    void removeStudent();
    void listStudents();
    void searchStudent();

    // === Attendance operations ===
    void markAttendance();
    void editAttendance();
    void viewAttendanceByDate();
    void viewAttendanceByStudent();
    void addComment();
    void viewComments();

    // === Report operations ===
    void showAnalytics();
    void showLowAttendance();
    void saveReports();

    // === Input helpers (static — no state required) ===
    static int             readInt   (const std::string& prompt,
                                      int min = std::numeric_limits<int>::min(),
                                      int max = std::numeric_limits<int>::max());
    static std::string     readString(const std::string& prompt);
    static Date            readDate  (const std::string& prompt);
    static AttendanceStatus readStatus(const std::string& prompt);

    // === Display helpers ===
    static void printSeparator      (char c = '=', int width = 60);
    static void printHeader         (const std::string& title);
    static void pressEnterToContinue();
};

} // namespace ams
