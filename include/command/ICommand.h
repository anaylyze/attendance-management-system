#pragma once

#include <string>

namespace ams {

/**
 * Command interface for the undo/redo subsystem.
 * Each concrete command encapsulates a single reversible mutation
 * of AttendanceManager's state.
 *
 * Contract:
 *   execute() applies the change.
 *   undo()    applies the inverse change.
 *   Both must be idempotent when called in the correct sequence.
 */
class ICommand {
public:
    virtual ~ICommand() = default;

    virtual void        execute()     = 0;
    virtual void        undo()        = 0;
    virtual std::string description() const = 0;
};

} // namespace ams
