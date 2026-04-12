#include "manager/AttendanceManager.h"
#include "command/MarkAttendanceCommand.h"
#include "command/EditAttendanceCommand.h"
#include "service/Logger.h"
#include "util/Utils.h"

#include <algorithm>
#include <stdexcept>
#include <memory>

namespace ams {

const std::vector<AttendanceRecord> AttendanceManager::EMPTY_RECORDS{};

void AttendanceManager::markAttendance(int studentId, const Date& date,
                                       AttendanceStatus status, int teacherId) {
    if (hasRecord(studentId, date)) {
        throw std::runtime_error(
            "Attendance already marked for student #" + std::to_string(studentId) +
            " on " + date.toString() + ". Use 'Edit Attendance' to modify.");
    }
    AttendanceRecord record(studentId, date, status, teacherId, util::currentTimestamp());
    auto cmd = std::unique_ptr<ICommand>(new MarkAttendanceCommand(*this, record));
    undoRedoStack_.execute(std::move(cmd));
    Logger::instance().info(
        "MARK: student=" + std::to_string(studentId) +
        " date=" + date.toString() +
        " status=" + statusToString(status));
}

void AttendanceManager::editAttendance(int studentId, const Date& date,
                                       AttendanceStatus newStatus, int teacherId) {
    if (!hasRecord(studentId, date)) {
        throw std::runtime_error(
            "No attendance record for student #" + std::to_string(studentId) +
            " on " + date.toString());
    }
    const auto& dayRecords = records_.at(date);
    AttendanceRecord oldRec;
    for (const auto& r : dayRecords) {
        if (r.studentId == studentId) { oldRec = r; break; }
    }
    AttendanceRecord newRec(studentId, date, newStatus, teacherId, util::currentTimestamp());
    auto cmd = std::unique_ptr<ICommand>(
        new EditAttendanceCommand(*this, oldRec, newRec));
    undoRedoStack_.execute(std::move(cmd));
    Logger::instance().info(
        "EDIT: student=" + std::to_string(studentId) +
        " newStatus=" + statusToString(newStatus));
}

bool AttendanceManager::hasRecord(int studentId, const Date& date) const noexcept {
    auto it = records_.find(date);
    if (it == records_.end()) return false;
    for (const auto& r : it->second) {
        if (r.studentId == studentId) return true;
    }
    return false;
}

const std::vector<AttendanceRecord>&
AttendanceManager::recordsForDate(const Date& date) const {
    auto it = records_.find(date);
    return it != records_.end() ? it->second : EMPTY_RECORDS;
}

std::vector<AttendanceRecord>
AttendanceManager::recordsForStudent(int studentId) const {
    std::vector<AttendanceRecord> result;
    for (auto it = records_.begin(); it != records_.end(); ++it) {
        for (const auto& r : it->second) {
            if (r.studentId == studentId) result.push_back(r);
        }
    }
    std::sort(result.begin(), result.end(),
              [](const AttendanceRecord& a, const AttendanceRecord& b) {
                  return a.date < b.date;
              });
    return result;
}

void AttendanceManager::addComment(Comment comment) {
    comments_.push_back(std::move(comment));
}

std::vector<Comment>
AttendanceManager::commentsFor(int studentId, const Date& date) const {
    std::vector<Comment> result;
    for (const auto& c : comments_) {
        if (c.studentId == studentId && c.date == date) result.push_back(c);
    }
    return result;
}

std::vector<Comment>
AttendanceManager::allCommentsForStudent(int studentId) const {
    std::vector<Comment> result;
    for (const auto& c : comments_) {
        if (c.studentId == studentId) result.push_back(c);
    }
    return result;
}

void AttendanceManager::undo() {
    std::string desc = undoRedoStack_.undoDescription();
    undoRedoStack_.undo();
    Logger::instance().info("UNDO: " + desc);
}

void AttendanceManager::redo() {
    std::string desc = undoRedoStack_.redoDescription();
    undoRedoStack_.redo();
    Logger::instance().info("REDO: " + desc);
}

void AttendanceManager::addRecordInternal(const AttendanceRecord& record) {
    records_[record.date].push_back(record);
}

void AttendanceManager::removeRecordInternal(const AttendanceRecord& record) {
    auto it = records_.find(record.date);
    if (it == records_.end()) return;
    auto& vec = it->second;
    vec.erase(
        std::remove_if(vec.begin(), vec.end(),
                       [&record](const AttendanceRecord& r) {
                           return r.studentId == record.studentId &&
                                  r.date      == record.date;
                       }),
        vec.end());
    if (vec.empty()) records_.erase(it);
}

void AttendanceManager::replaceRecordInternal(const AttendanceRecord& oldRec,
                                              const AttendanceRecord& newRec) {
    removeRecordInternal(oldRec);
    addRecordInternal(newRec);
}

} // namespace ams
