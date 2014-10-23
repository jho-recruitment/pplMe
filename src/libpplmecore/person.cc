/**
 *  @file
 *  @brief   Implementation for pplme::core::Person.
 *  @author  j.ho
 */


#include <boost/uuid/nil_generator.hpp>
#include "person.h"


namespace pplme {
namespace core {


Person::Person() :
    id_{boost::uuids::nil_generator()()},
    name_{""},
    date_of_birth_{},
    location_of_home_{}
{}


Person::Person(PersonId id,
               std::string name,
               boost::gregorian::date date_of_birth,
               GeoPosition location_of_home) :
    id_{id},
    name_{std::move(name)},
    date_of_birth_{date_of_birth},
    location_of_home_{location_of_home}
{}


}  // namespace core
}  // namespace pplme
