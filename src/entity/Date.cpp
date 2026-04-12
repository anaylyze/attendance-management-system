#include "entity/Date.h"

#include <sstream>
#include <iomanip>
#include <ctime>
#include <stdexcept>

namespace ams {

// ──────────────────────────────────────────────────────────────────────────────
// Construction
// ──────────────────────────────────────────────────────────────────────────────

Date::Date(int year, int month, int day)
    : year_{year}, month_{month}, day_{day}
{
    if (!isValid()) {
        throw std::invalid_argument(
            "Invalid date: " + std::to_string(year) + "-" +
            std::to_string(month) + "-" + std::to_string(day));
    }
}

Date::Date(const std::string& iso) {
    if (iso.size() != 10 || iso[4] != '-' || iso[7] != '-') {
        throw std::invalid_argument(
            "Date must be in YYYY-MM-DD format, got: '" + iso + "'");
    }
    try {
        year_  = std::stoi(iso.substr(0, 4));
        month_ = std::stoi(iso.substr(5, 2));
        day_   = std::stoi(iso.substr(8, 2));
    } catch (const std::exception&) {
        throw std::invalid_argument("Non-numeric characters in date string: '" + iso + "'");
    }
    if (!isValid()) {
        throw std::invalid_argument("Date value out of range: '" + iso + "'");
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// Factory
// ──────────────────────────────────────────────────────────────────────────────

Date Date::today() {
    std::time_t t = std::time(nullptr);
    std::tm tm_buf{};
    std::tm* ptm = std::localtime(&t);
    if (ptm) tm_buf = *ptm;
    return Date(tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday);
}

// ──────────────────────────────────────────────────────────────────────────────
// Formatting
// ──────────────────────────────────────────────────────────────────────────────

std::string Date::toString() const {
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(4) << year_  << '-'
        << std::setw(2) << month_ << '-'
        << std::setw(2) << day_;
    return oss.str();
}

// ──────────────────────────────────────────────────────────────────────────────
// Validation helpers
// ──────────────────────────────────────────────────────────────────────────────

bool Date::isValid() const noexcept {
    if (year_ < 1 || month_ < 1 || month_ > 12 || day_ < 1) return false;
    return day_ <= daysInMonth(year_, month_);
}

bool Date::isLeapYear(int y) noexcept {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

int Date::daysInMonth(int y, int m) noexcept {
    constexpr int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m == 2 && isLeapYear(y)) return 29;
    return days[m];
}

// ──────────────────────────────────────────────────────────────────────────────
// Comparators
// ──────────────────────────────────────────────────────────────────────────────

bool Date::operator==(const Date& o) const noexcept {
    return year_ == o.year_ && month_ == o.month_ && day_ == o.day_;
}
bool Date::operator!=(const Date& o) const noexcept { return !(*this == o); }
bool Date::operator< (const Date& o) const noexcept {
    if (year_  != o.year_)  return year_  < o.year_;
    if (month_ != o.month_) return month_ < o.month_;
    return day_ < o.day_;
}
bool Date::operator<=(const Date& o) const noexcept { return !(o < *this); }
bool Date::operator> (const Date& o) const noexcept { return o < *this; }
bool Date::operator>=(const Date& o) const noexcept { return !(*this < o); }

} // namespace ams
