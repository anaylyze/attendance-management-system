#pragma once

#include "entity/AttendanceRecord.h"
#include "entity/Comment.h"
#include "entity/Date.h"
#include "service/UndoRedoStack.h"
#include <unordered_map>
#include <vector>
#include <string>

namespace ams {

/**
 * Core manager for all attendance state.
 *
 * Primary store: unordered_map<Date, vector<AttendanceRecord>>
 *   - O(1) average date-keyed retrieval via std::hash<Date> specialization.
 *   - vector<AttendanceRecord> preserves per-day insertion order and is
 *     cache-friendly for iteration.
 *
 * Comments store: vector<Comment> (flat, filtered on read)
 *   - Comments are relatively rare and small; linear scan is negligible.
 *   - Avoids the complexity of a nested multimap structure.
 *
 * Undo/Redo: all mutations go through UndoRedoStack via Command objects.
 *   The *Internal methods are the only direct write paths, called exclusively
 *   by MarkAttendanceCommand and EditAttendanceCommand.
 */
class AttendanceManager {
public:
    // === Public Mutation API ===

    /**
     * Mark attendance for a student on a given date.
     * Throws std::runtime_error if a record already exists (duplicate guard).
     * Creates a MarkAttendanceCommand and executes it via UndoRedoStack.
     */
    void markAttendance(int studentId, const Date& date,
                        AttendanceStatus status, int teacherId);

    /**
     * Edit an existing attendance record's status.
     * Throws std::runtime_error if no record exists for (studentId, date).
     * Creates an EditAttendanceCommand and executes it via UndoRedoStack.
     */
    void editAttendance(int studentId, const Date& date,
                        AttendanceStatus newStatus, int teacherId);

    // === Query API ===

    bool hasRecord(int studentId, const Date& date) const noexcept;

    /// Returns a const-ref to the day's records. Returns EMPTY_RECORDS if no entry.
    const std::vector<AttendanceRecord>& recordsForDate(const Date& date) const;

    /// Returns all records for a student, sorted by date ascending. O(n) scan.
    std::vector<AttendanceRecord> recordsForStudent(int studentId) const;

    /// Read-only access to the full store (for serialization and analytics).
    const std::unordered_map<Date, std::vector<AttendanceRecord>>&
    allRecords() const noexcept { return records_; }

    // === Comment API ===
    void addComment(Comment comment);
    std::vector<Comment> commentsFor(int studentId, const Date& date) const;
    std::vector<Comment> allCommentsForStudent(int studentId) const;
    const std::vector<Comment>& allComments() const noexcept { return comments_; }

    // === Undo/Redo ===
    void undo();
    void redo();
    bool        canUndo()          const noexcept { return undoRedoStack_.canUndo(); }
    bool        canRedo()          const noexcept { return undoRedoStack_.canRedo(); }
    std::string undoDescription()  const          { return undoRedoStack_.undoDescription(); }
    std::string redoDescription()  const          { return undoRedoStack_.redoDescription(); }

    // === Internal write API — called ONLY by Command objects ===
    void addRecordInternal    (const AttendanceRecord& record);
    void removeRecordInternal (const AttendanceRecord& record);
    void replaceRecordInternal(const AttendanceRecord& oldRec,
                               const AttendanceRecord& newRec);

    void clear() {
        records_.clear();
        comments_.clear();
        undoRedoStack_.clear();
    }

private:
    std::unordered_map<Date, std::vector<AttendanceRecord>> records_;
    std::vector<Comment>                                    comments_;
    UndoRedoStack                                           undoRedoStack_;

    static const std::vector<AttendanceRecord> EMPTY_RECORDS;
};

} // namespace ams
