#include "entity/Teacher.h"

#include <sstream>
#include <vector>
#include <stdexcept>

namespace ams {

Teacher::Teacher(int id, std::string name, std::string subject, std::string department)
    : Person{id, std::move(name)},
      subject_{std::move(subject)},
      department_{std::move(department)}
{}

// CSV format: id,name,TEACHER,subject,department
std::string Teacher::toCSVRow() const {
    return std::to_string(id_) + "," +
           name_               + "," +
           "TEACHER"           + "," +
           subject_            + "," +
           department_;
}

Teacher Teacher::fromCSVRow(const std::string& line) {
    std::istringstream ss(line);
    std::string token;
    std::vector<std::string> f;
    while (std::getline(ss, token, ',')) f.push_back(token);

    if (f.size() < 5) {
        throw std::runtime_error(
            "Teacher CSV row has " + std::to_string(f.size()) +
            " fields (expected 5): '" + line + "'");
    }
    return Teacher{std::stoi(f[0]), f[1], f[3], f[4]};
}

} // namespace ams
