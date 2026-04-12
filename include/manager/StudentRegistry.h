#pragma once

#include "entity/Student.h"
#include <map>
#include <vector>
#include <string>

namespace ams {

/**
 * Central registry for all students.
 *
 * Data structure: std::map<int, Student>
 * Rationale:
 *   - O(log n) lookup by student ID (natural key).
 *   - Ordered iteration by ID simplifies generateId() — O(1) via rbegin().
 *   - Supports range scans (e.g., by section) via linear iteration.
 *   - unordered_map would give O(1) average but loses ordering guarantees;
 *     the small constant benefit is not worth the predictability trade-off
 *     for datasets in the thousands.
 */
class StudentRegistry {
public:
    /// Add a student. Throws std::runtime_error if ID already exists.
    void addStudent(Student student);

    /// Remove by ID. Returns false if not found.
    bool removeStudent(int id);

    /// Throws std::out_of_range if not found.
    const Student& getStudent(int id) const;
    Student&       getStudent(int id);

    /// Non-throwing lookup: returns nullptr if not found.
    const Student* findStudent(int id) const noexcept;

    bool   exists(int id) const noexcept;
    size_t count()        const noexcept { return students_.size(); }

    /// Ordered view of all students (read-only).
    const std::map<int, Student>& all() const noexcept { return students_; }

    /// Returns max existing ID + 1, or 1 if empty. O(1) via rbegin().
    int generateId() const noexcept;

    /// Filter by exact section name.
    std::vector<const Student*> bySection(const std::string& section) const;

    /// Case-insensitive substring search on student name.
    std::vector<const Student*> searchByName(const std::string& query) const;

    void clear() { students_.clear(); }

private:
    std::map<int, Student> students_;
};

} // namespace ams
