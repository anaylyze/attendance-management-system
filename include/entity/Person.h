#pragma once

#include <string>

namespace ams {

/**
 * Abstract base for all people in the system.
 * Enforces identity contract: every person has a positive integer ID and a name.
 * Derived classes must implement role() and toCSVRow() for polymorphic dispatch.
 */
class Person {
public:
    Person(int id, std::string name);
    virtual ~Person() = default;

    Person(const Person&) = default;
    Person& operator=(const Person&) = default;
    Person(Person&&) = default;
    Person& operator=(Person&&) = default;

    int                id()   const noexcept { return id_; }
    const std::string& name() const noexcept { return name_; }

    void setName(const std::string& name);

    virtual std::string role()     const = 0; ///< "STUDENT" or "TEACHER"
    virtual std::string toCSVRow() const = 0; ///< Serialization hook

    bool operator==(const Person& o) const noexcept { return id_ == o.id_; }
    bool operator!=(const Person& o) const noexcept { return id_ != o.id_; }

protected:
    int         id_;
    std::string name_;
};

} // namespace ams
