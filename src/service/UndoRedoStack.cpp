#include "service/UndoRedoStack.h"
#include <stdexcept>

namespace ams {

void UndoRedoStack::execute(std::unique_ptr<ICommand> cmd) {
    // Apply the command's effect first
    cmd->execute();

    // Any new action invalidates the entire redo future
    while (!redoStack_.empty()) redoStack_.pop();

    undoStack_.push(std::move(cmd));
}

void UndoRedoStack::undo() {
    if (undoStack_.empty()) {
        throw std::runtime_error("Nothing to undo");
    }
    // 1. Call undo on the top command (still owned by undoStack_)
    undoStack_.top()->undo();
    // 2. Transfer ownership to the redo stack
    redoStack_.push(std::move(undoStack_.top()));
    // 3. Pop the now-empty unique_ptr from the undo stack
    undoStack_.pop();
}

void UndoRedoStack::redo() {
    if (redoStack_.empty()) {
        throw std::runtime_error("Nothing to redo");
    }
    // 1. Re-execute the command
    redoStack_.top()->execute();
    // 2. Transfer back to undo stack
    undoStack_.push(std::move(redoStack_.top()));
    // 3. Pop the moved-from entry
    redoStack_.pop();
}

std::string UndoRedoStack::undoDescription() const {
    return undoStack_.empty() ? "(nothing to undo)" : undoStack_.top()->description();
}

std::string UndoRedoStack::redoDescription() const {
    return redoStack_.empty() ? "(nothing to redo)" : redoStack_.top()->description();
}

void UndoRedoStack::clear() {
    while (!undoStack_.empty()) undoStack_.pop();
    while (!redoStack_.empty()) redoStack_.pop();
}

} // namespace ams
