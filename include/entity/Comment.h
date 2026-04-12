#pragma once

#include "entity/Date.h"
#include <string>

namespace ams {

/**
 * A textual remark added by a teacher for a specific student on a specific date.
 * Multiple comments per (studentId, date) pair are supported.
 */
struct Comment {
    int         studentId{0};
    Date        date{};
    int         authorId{0};  ///< Teacher ID
    std::string text;
    std::string timestamp;    ///< ISO 8601 creation time

    Comment() = default;
    Comment(int sid, Date d, int aid, std::string txt, std::string ts);

    std::string toCSVRow() const;
    static Comment fromCSVRow(const std::string& line);
};

} // namespace ams
