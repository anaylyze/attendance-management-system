#pragma once

#include "entity/Person.h"
#include <string>

namespace ams {

class Student final : public Person {
public:
    Student(int id, std::string name, std::string section,
            int enrollmentYear, std::string rollNumber);

    const std::string& section()        const noexcept { return section_; }
    int                enrollmentYear() const noexcept { return enrollmentYear_; }
    const std::string& rollNumber()     const noexcept { return rollNumber_; }

    void setSection(const std::string& s) { section_ = s; }

    std::string role()     const override { return "STUDENT"; }
    std::string toCSVRow() const override;

    /// Factory: construct from a CSV data row (no header).
    static Student fromCSVRow(const std::string& line);

private:
    std::string section_;
    int         enrollmentYear_;
    std::string rollNumber_;
};

} // namespace ams
