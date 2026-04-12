#include "command/EditAttendanceCommand.h"
#include "manager/AttendanceManager.h"

namespace ams {

EditAttendanceCommand::EditAttendanceCommand(AttendanceManager& mgr,
                                             AttendanceRecord   oldRecord,
                                             AttendanceRecord   newRecord)
    : mgr_{mgr},
      oldRecord_{std::move(oldRecord)},
      newRecord_{std::move(newRecord)}
{}

void EditAttendanceCommand::execute() {
    mgr_.replaceRecordInternal(oldRecord_, newRecord_);
}

void EditAttendanceCommand::undo() {
    mgr_.replaceRecordInternal(newRecord_, oldRecord_);
}

std::string EditAttendanceCommand::description() const {
    return "Edit student #" + std::to_string(newRecord_.studentId) +
           " on " + newRecord_.date.toString() +
           ": " + statusToString(oldRecord_.status) +
           " -> " + statusToString(newRecord_.status);
}

} // namespace ams
