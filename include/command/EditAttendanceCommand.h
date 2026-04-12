#pragma once

#include "command/ICommand.h"
#include "entity/AttendanceRecord.h"

namespace ams {

class AttendanceManager; // forward declaration

/**
 * Encapsulates an "edit attendance" operation.
 * Stores both old and new records for bidirectional reversal.
 * execute() → replaceRecordInternal(old, new)
 * undo()    → replaceRecordInternal(new, old)
 */
class EditAttendanceCommand final : public ICommand {
public:
    EditAttendanceCommand(AttendanceManager& mgr,
                          AttendanceRecord   oldRecord,
                          AttendanceRecord   newRecord);

    void        execute()     override;
    void        undo()        override;
    std::string description() const override;

private:
    AttendanceManager& mgr_;
    AttendanceRecord   oldRecord_;
    AttendanceRecord   newRecord_;
};

} // namespace ams
