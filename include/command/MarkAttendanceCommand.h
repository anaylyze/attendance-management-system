#pragma once

#include "command/ICommand.h"
#include "entity/AttendanceRecord.h"

namespace ams {

// Forward declaration — breaks the include cycle between
// AttendanceManager.h → UndoRedoStack.h → ICommand.h ← MarkAttendanceCommand.h → AttendanceManager.h
class AttendanceManager;

/**
 * Encapsulates a "mark attendance" operation.
 * execute() → addRecordInternal(record_)
 * undo()    → removeRecordInternal(record_)
 */
class MarkAttendanceCommand final : public ICommand {
public:
    MarkAttendanceCommand(AttendanceManager& mgr, AttendanceRecord record);

    void        execute()     override;
    void        undo()        override;
    std::string description() const override;

private:
    AttendanceManager& mgr_;
    AttendanceRecord   record_;
};

} // namespace ams
