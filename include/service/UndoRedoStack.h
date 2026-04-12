#pragma once

#include "command/ICommand.h"
#include <stack>
#include <memory>
#include <string>
#include <stdexcept>

namespace ams {

/**
 * Two-stack undo/redo manager using the Command pattern.
 *
 * Invariant: executing a new command always clears the redo stack,
 * which is the standard undo/redo contract (a new action "forks" history).
 *
 * Ownership: commands are owned exclusively via unique_ptr; no raw pointers escape.
 *
 * MAX_HISTORY is a soft guideline — the stack is not actively pruned
 * because std::stack has no O(1) access to the bottom. For a hard cap,
 * replace std::stack with std::deque.
 */
class UndoRedoStack {
public:
    static constexpr size_t MAX_HISTORY = 100;

    /// Execute a command and push it onto the undo stack. Clears redo stack.
    void execute(std::unique_ptr<ICommand> cmd);

    bool canUndo() const noexcept { return !undoStack_.empty(); }
    bool canRedo() const noexcept { return !redoStack_.empty(); }

    void undo(); ///< Throws std::runtime_error if nothing to undo
    void redo(); ///< Throws std::runtime_error if nothing to redo

    std::string undoDescription() const;
    std::string redoDescription() const;

    size_t undoDepth() const noexcept { return undoStack_.size(); }
    size_t redoDepth() const noexcept { return redoStack_.size(); }

    void clear();

private:
    std::stack<std::unique_ptr<ICommand>> undoStack_;
    std::stack<std::unique_ptr<ICommand>> redoStack_;
};

} // namespace ams
