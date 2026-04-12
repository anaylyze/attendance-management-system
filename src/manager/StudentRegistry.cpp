#include "manager/StudentRegistry.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace ams {

void StudentRegistry::addStudent(Student student) {
    int id = student.id();
    if (students_.count(id)) {
        throw std::runtime_error(
            "Student with ID " + std::to_string(id) + " already exists");
    }
    students_.emplace(id, std::move(student));
}

bool StudentRegistry::removeStudent(int id) {
    return students_.erase(id) > 0;
}

const Student& StudentRegistry::getStudent(int id) const {
    auto it = students_.find(id);
    if (it == students_.end()) {
        throw std::out_of_range("Student not found: ID=" + std::to_string(id));
    }
    return it->second;
}

Student& StudentRegistry::getStudent(int id) {
    auto it = students_.find(id);
    if (it == students_.end()) {
        throw std::out_of_range("Student not found: ID=" + std::to_string(id));
    }
    return it->second;
}

const Student* StudentRegistry::findStudent(int id) const noexcept {
    auto it = students_.find(id);
    return it != students_.end() ? &it->second : nullptr;
}

bool StudentRegistry::exists(int id) const noexcept {
    return students_.count(id) > 0;
}

// O(1) — std::map is ordered; max key is at rbegin()
int StudentRegistry::generateId() const noexcept {
    if (students_.empty()) return 1;
    return students_.rbegin()->first + 1;
}

std::vector<const Student*> StudentRegistry::bySection(const std::string& section) const {
    std::vector<const Student*> result;
    for (auto it = students_.begin(); it != students_.end(); ++it) {
        if (it->second.section() == section) result.push_back(&it->second);
    }
    return result;
}

std::vector<const Student*> StudentRegistry::searchByName(const std::string& query) const {
    // Build lowercase query once
    std::string lq = query;
    std::transform(lq.begin(), lq.end(), lq.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    std::vector<const Student*> result;
    for (auto it = students_.begin(); it != students_.end(); ++it) {
        std::string ln = it->second.name();
        std::transform(ln.begin(), ln.end(), ln.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (ln.find(lq) != std::string::npos) {
            result.push_back(&it->second);
        }
    }
    return result;
}

} // namespace ams
