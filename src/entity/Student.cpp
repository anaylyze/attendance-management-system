#include "entity/Student.h"

#include <sstream>
#include <vector>
#include <stdexcept>

namespace ams {

Student::Student(int id, std::string name, std::string section,
                 int enrollmentYear, std::string rollNumber)
    : Person{id, std::move(name)},
      section_{std::move(section)},
      enrollmentYear_{enrollmentYear},
      rollNumber_{std::move(rollNumber)}
{
    if (section_.empty())    throw std::invalid_argument("Section cannot be empty");
    if (rollNumber_.empty()) throw std::invalid_argument("Roll number cannot be empty");
    if (enrollmentYear_ < 1900 || enrollmentYear_ > 2200)
        throw std::invalid_argument("Enrollment year out of range");
}

// CSV format: id,name,STUDENT,section,enrollmentYear,rollNumber
std::string Student::toCSVRow() const {
    return std::to_string(id_) + "," +
           name_                      + "," +
           "STUDENT"                  + "," +
           section_                   + "," +
           std::to_string(enrollmentYear_) + "," +
           rollNumber_;
}

Student Student::fromCSVRow(const std::string& line) {
    std::istringstream ss(line);
    std::string token;
    std::vector<std::string> f;
    while (std::getline(ss, token, ',')) f.push_back(token);

    if (f.size() < 6) {
        throw std::runtime_error(
            "Student CSV row has " + std::to_string(f.size()) +
            " fields (expected 6): '" + line + "'");
    }
    return Student{
        std::stoi(f[0]),  // id
        f[1],             // name
        f[3],             // section
        std::stoi(f[4]),  // enrollmentYear
        f[5]              // rollNumber
    };
}

} // namespace ams
