#include "command/MarkAttendanceCommand.h"
#include "manager/AttendanceManager.h"

namespace ams {

MarkAttendanceCommand::MarkAttendanceCommand(AttendanceManager& mgr, AttendanceRecord record)
    : mgr_{mgr}, record_{std::move(record)}
{}

void MarkAttendanceCommand::execute() {
    mgr_.addRecordInternal(record_);
}

void MarkAttendanceCommand::undo() {
    mgr_.removeRecordInternal(record_);
}

std::string MarkAttendanceCommand::description() const {
    return "Mark " + statusToString(record_.status) +
           " for student #" + std::to_string(record_.studentId) +
           " on " + record_.date.toString();
}

} // namespace ams
