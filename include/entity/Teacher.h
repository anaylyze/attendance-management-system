#pragma once

#include "entity/Person.h"
#include <string>

namespace ams {

class Teacher final : public Person {
public:
    Teacher(int id, std::string name, std::string subject, std::string department);

    const std::string& subject()    const noexcept { return subject_; }
    const std::string& department() const noexcept { return department_; }

    std::string role()     const override { return "TEACHER"; }
    std::string toCSVRow() const override;

    static Teacher fromCSVRow(const std::string& line);

private:
    std::string subject_;
    std::string department_;
};

} // namespace ams
