#include "service/FileManager.h"
#include "service/Logger.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>   // mkdir fallback for GCC 6 / MinGW

#ifdef _WIN32
#  include <direct.h>   // _mkdir on Windows
#  define AMS_MKDIR(p) _mkdir(p)
#else
#  include <sys/types.h>
#  define AMS_MKDIR(p) mkdir((p), 0755)
#endif

namespace ams {

FileManager::FileManager(std::string dataDir)
    : dataDir_{std::move(dataDir)}
{}

void FileManager::ensureDataDir() {
    AMS_MKDIR(dataDir_.c_str()); // no-op if already exists
}

std::string FileManager::studentsPath()   const { return dataDir_ + "/students.csv";   }
std::string FileManager::attendancePath() const { return dataDir_ + "/attendance.csv"; }
std::string FileManager::commentsPath()   const { return dataDir_ + "/comments.csv";   }

std::vector<std::string> FileManager::splitCSV(const std::string& line) {
    std::vector<std::string> fields;
    std::istringstream ss(line);
    std::string token;
    while (std::getline(ss, token, ',')) {
        fields.push_back(token);
    }
    return fields;
}

// ─────────────────────────────────────────────────────────────────────────────
// Students
// ─────────────────────────────────────────────────────────────────────────────

void FileManager::saveStudents(const StudentRegistry& registry) {
    ensureDataDir();
    std::ofstream f(studentsPath());
    if (!f.is_open()) throw std::runtime_error("Cannot open: " + studentsPath());

    f << "id,name,role,section,enrollmentYear,rollNumber\n";
    for (auto it = registry.all().begin(); it != registry.all().end(); ++it) {
        f << it->second.toCSVRow() << "\n";
    }
    Logger::instance().info(
        "Saved " + std::to_string(registry.count()) + " students -> " + studentsPath());
}

void FileManager::loadStudents(StudentRegistry& registry) {
    registry.clear();
    std::ifstream f(studentsPath());
    if (!f.is_open()) {
        Logger::instance().warning("Students file not found: " + studentsPath());
        return;
    }
    std::string line;
    std::getline(f, line); // consume header

    int loaded = 0, skipped = 0;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        try {
            registry.addStudent(Student::fromCSVRow(line));
            ++loaded;
        } catch (const std::exception& e) {
            Logger::instance().warning(
                "Skipping malformed student row: '" + line + "' - " + e.what());
            ++skipped;
        }
    }
    Logger::instance().info(
        "Loaded " + std::to_string(loaded) + " students (" +
        std::to_string(skipped) + " skipped)");
}

// ─────────────────────────────────────────────────────────────────────────────
// Attendance
// ─────────────────────────────────────────────────────────────────────────────

void FileManager::saveAttendance(const AttendanceManager& mgr) {
    ensureDataDir();
    std::ofstream f(attendancePath());
    if (!f.is_open()) throw std::runtime_error("Cannot open: " + attendancePath());

    f << "studentId,date,status,teacherId,timestamp\n";
    int count = 0;
    const auto& allRecs = mgr.allRecords();
    for (auto it = allRecs.begin(); it != allRecs.end(); ++it) {
        for (const auto& r : it->second) {
            f << r.toCSVRow() << "\n";
            ++count;
        }
    }
    Logger::instance().info(
        "Saved " + std::to_string(count) + " attendance records -> " + attendancePath());
}

void FileManager::loadAttendance(AttendanceManager& mgr) {
    std::ifstream f(attendancePath());
    if (!f.is_open()) {
        Logger::instance().warning("Attendance file not found: " + attendancePath());
        return;
    }
    std::string line;
    std::getline(f, line); // consume header

    int loaded = 0, skipped = 0;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        try {
            mgr.addRecordInternal(AttendanceRecord::fromCSVRow(line));
            ++loaded;
        } catch (const std::exception& e) {
            Logger::instance().warning(
                "Skipping malformed attendance row: '" + line + "' - " + e.what());
            ++skipped;
        }
    }
    Logger::instance().info(
        "Loaded " + std::to_string(loaded) + " attendance records (" +
        std::to_string(skipped) + " skipped)");
}

// ─────────────────────────────────────────────────────────────────────────────
// Comments
// ─────────────────────────────────────────────────────────────────────────────

void FileManager::saveComments(const AttendanceManager& mgr) {
    ensureDataDir();
    std::ofstream f(commentsPath());
    if (!f.is_open()) throw std::runtime_error("Cannot open: " + commentsPath());

    f << "studentId,date,authorId,text,timestamp\n";
    int count = 0;
    for (const auto& c : mgr.allComments()) {
        f << c.toCSVRow() << "\n";
        ++count;
    }
    Logger::instance().info(
        "Saved " + std::to_string(count) + " comments -> " + commentsPath());
}

void FileManager::loadComments(AttendanceManager& mgr) {
    std::ifstream f(commentsPath());
    if (!f.is_open()) {
        Logger::instance().warning("Comments file not found: " + commentsPath());
        return;
    }
    std::string line;
    std::getline(f, line); // consume header

    int loaded = 0, skipped = 0;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        try {
            mgr.addComment(Comment::fromCSVRow(line));
            ++loaded;
        } catch (const std::exception& e) {
            Logger::instance().warning(
                "Skipping malformed comment row: '" + line + "' - " + e.what());
            ++skipped;
        }
    }
    Logger::instance().info(
        "Loaded " + std::to_string(loaded) + " comments (" +
        std::to_string(skipped) + " skipped)");
}

// ─────────────────────────────────────────────────────────────────────────────
// Aggregates
// ─────────────────────────────────────────────────────────────────────────────

void FileManager::saveAll(const StudentRegistry& registry, const AttendanceManager& mgr) {
    ensureDataDir();
    saveStudents(registry);
    saveAttendance(mgr);
    saveComments(mgr);
}

void FileManager::loadAll(StudentRegistry& registry, AttendanceManager& mgr) {
    mgr.clear();
    loadStudents(registry);
    loadAttendance(mgr);
    loadComments(mgr);
}

} // namespace ams
