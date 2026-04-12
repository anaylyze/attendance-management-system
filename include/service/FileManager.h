#pragma once

#include "manager/StudentRegistry.h"
#include "manager/AttendanceManager.h"
#include <string>
#include <vector>

namespace ams {

/**
 * Handles all persistent I/O using CSV format.
 *
 * File layout:
 *   data/students.csv   → id,name,role,section,enrollmentYear,rollNumber
 *   data/attendance.csv → studentId,date,status,teacherId,timestamp
 *   data/comments.csv   → studentId,date,authorId,text,timestamp
 *
 * Corruption handling:
 *   - Each row is parsed in an isolated try/catch block.
 *   - Malformed rows (wrong column count, invalid values) are skipped
 *     and logged as warnings — never fatal to the load operation.
 *   - The header line is validated before reading data rows.
 *
 * CSV escaping:
 *   - Comment text has embedded commas replaced with ';' before saving.
 *   - Names containing commas would need quoting — currently assumed
 *     to be plain text (extendable to RFC 4180 with a proper CSV parser).
 */
class FileManager {
public:
    explicit FileManager(std::string dataDir);

    void saveStudents  (const StudentRegistry&   registry);
    void loadStudents  (StudentRegistry&          registry);

    void saveAttendance(const AttendanceManager& mgr);
    void loadAttendance(AttendanceManager&        mgr);

    void saveComments  (const AttendanceManager& mgr);
    void loadComments  (AttendanceManager&        mgr);

    /// Convenience: save all three files in one call.
    void saveAll(const StudentRegistry& registry, const AttendanceManager& mgr);

    /// Convenience: clear existing state and load all three files.
    void loadAll(StudentRegistry& registry, AttendanceManager& mgr);

    const std::string& dataDir() const noexcept { return dataDir_; }

    /// Creates dataDir_ if it does not already exist.
    void ensureDataDir();

private:
    std::string dataDir_;

    std::string studentsPath()   const;
    std::string attendancePath() const;
    std::string commentsPath()   const;

    static std::vector<std::string> splitCSV(const std::string& line);
};

} // namespace ams
