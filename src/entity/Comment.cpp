#include "entity/Comment.h"

#include <sstream>
#include <vector>
#include <algorithm>
#include <stdexcept>

namespace ams {

Comment::Comment(int sid, Date d, int aid, std::string txt, std::string ts)
    : studentId{sid},
      date{std::move(d)},
      authorId{aid},
      text{std::move(txt)},
      timestamp{std::move(ts)}
{}

// CSV format: studentId,date,authorId,text,timestamp
// NOTE: embedded commas in 'text' are replaced with ';' to preserve column count.
std::string Comment::toCSVRow() const {
    std::string safeText = text;
    std::replace(safeText.begin(), safeText.end(), ',', ';');
    return std::to_string(studentId) + "," +
           date.toString()           + "," +
           std::to_string(authorId)  + "," +
           safeText                  + "," +
           timestamp;
}

Comment Comment::fromCSVRow(const std::string& line) {
    std::istringstream ss(line);
    std::string token;
    std::vector<std::string> f;
    while (std::getline(ss, token, ',')) f.push_back(token);

    // Minimum 5 fields: sid, date, authorId, text, timestamp
    if (f.size() < 5) {
        throw std::runtime_error(
            "Comment CSV row has " + std::to_string(f.size()) +
            " fields (expected >= 5): '" + line + "'");
    }

    // timestamp is always the last field; text is field[3].
    // If text contained a ';', it was never split — safe.
    return Comment{
        std::stoi(f[0]),  // studentId
        Date{f[1]},       // date
        std::stoi(f[2]),  // authorId
        f[3],             // text
        f[4]              // timestamp
    };
}

} // namespace ams
