#include "entity/AttendanceRecord.h"

#include <sstream>
#include <vector>
#include <stdexcept>

namespace ams {

AttendanceRecord::AttendanceRecord(int sid, Date d, AttendanceStatus s, int tid, std::string ts)
    : studentId{sid},
      date{std::move(d)},
      status{s},
      teacherId{tid},
      timestamp{std::move(ts)}
{}

// CSV format: studentId,date,status,teacherId,timestamp
// e.g.:       101,2024-01-15,PRESENT,5,2024-01-15T09:00:00
std::string AttendanceRecord::toCSVRow() const {
    return std::to_string(studentId) + "," +
           date.toString()           + "," +
           statusToString(status)    + "," +
           std::to_string(teacherId) + "," +
           timestamp;
}

AttendanceRecord AttendanceRecord::fromCSVRow(const std::string& line) {
    std::istringstream ss(line);
    std::string token;
    std::vector<std::string> f;
    while (std::getline(ss, token, ',')) f.push_back(token);

    if (f.size() < 5) {
        throw std::runtime_error(
            "Attendance CSV row has " + std::to_string(f.size()) +
            " fields (expected 5): '" + line + "'");
    }
    return AttendanceRecord{
        std::stoi(f[0]),      // studentId
        Date{f[1]},           // date
        stringToStatus(f[2]), // status
        std::stoi(f[3]),      // teacherId
        f[4]                  // timestamp
    };
}

} // namespace ams
