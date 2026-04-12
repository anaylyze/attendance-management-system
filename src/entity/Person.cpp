#include "entity/Person.h"
#include <stdexcept>

namespace ams {

Person::Person(int id, std::string name)
    : id_{id}, name_{std::move(name)}
{
    if (id_ <= 0)      throw std::invalid_argument("Person ID must be a positive integer");
    if (name_.empty()) throw std::invalid_argument("Person name cannot be empty");
}

void Person::setName(const std::string& name) {
    if (name.empty()) throw std::invalid_argument("Name cannot be empty");
    name_ = name;
}

} // namespace ams
