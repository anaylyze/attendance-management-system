#pragma once

#include <string>
#include <stdexcept>
#include <functional>

namespace ams {

/**
 * Immutable value type representing a calendar date (YYYY-MM-DD).
 * Default-constructed Date is invalid (year=month=day=0).
 * Hashable for use as unordered_map/unordered_set key.
 */
class Date {
public:
    Date() = default;
    Date(int year, int month, int day);
    explicit Date(const std::string& iso); ///< Parses "YYYY-MM-DD"

    static Date today();

    int year()  const noexcept { return year_; }
    int month() const noexcept { return month_; }
    int day()   const noexcept { return day_; }

    std::string toString() const; ///< Returns "YYYY-MM-DD"
    bool isValid() const noexcept;

    bool operator==(const Date& o) const noexcept;
    bool operator!=(const Date& o) const noexcept;
    bool operator< (const Date& o) const noexcept;
    bool operator<=(const Date& o) const noexcept;
    bool operator> (const Date& o) const noexcept;
    bool operator>=(const Date& o) const noexcept;

private:
    int year_{0}, month_{0}, day_{0};

    static bool isLeapYear(int y) noexcept;
    static int  daysInMonth(int y, int m) noexcept;
};

} // namespace ams

// STL hash specialization — required for unordered containers
namespace std {
template <>
struct hash<ams::Date> {
    size_t operator()(const ams::Date& d) const noexcept {
        // Pack into a single dense integer: max value 99991231
        return std::hash<int>{}(d.year() * 10000 + d.month() * 100 + d.day());
    }
};
} // namespace std
