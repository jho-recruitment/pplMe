/**
 *  @file
 *  @brief   Encapsulation of the set of data required to identify a person
 *           and match them with similar people like them, according to the
 *           pplMe project requirements.
 *  @author  j.ho
 */
#ifndef PPLME_LIBPPLMECORE_PERSON_H_
#define PPLME_LIBPPLMECORE_PERSON_H_


#include <boost/date_time/gregorian/gregorian.hpp>
#include "geo_position.h"
#include "person_id.h"


namespace pplme {
namespace core {


/**
 *  @remarks
 *  Instances of Person are considered valid solely based on them having
 *  a valid (non-nil) id.
 */
class Person {
 public:
  /** Creates an invalid person. */
  Person();
  /** Creates a valid person, assuming that @a id is non-nil. */
  Person(PersonId id,
         std::string name,
         boost::gregorian::date date_of_birth,
         GeoPosition location_of_home);

  /** @return true iff id() is valid/non-nil. */
  bool IsValid() { return pplme::core::IsValid(id_); }

  PersonId const& id() const { return id_; }
  std::string const& name() const { return name_; }
  boost::gregorian::date date_of_birth() const { return date_of_birth_; }
  GeoPosition const& location_of_home() const { return location_of_home_; }
  
 private:
  /** Person's unique id. */
  PersonId id_;
  /** Person's name encoded using UTF-8. */
  std::string name_;
  /** Person's date of birth. */
  boost::gregorian::date date_of_birth_;
  /** Where the person lives. */
  GeoPosition location_of_home_;
};


}  // namespace core
}  // namespace pplme


#endif  // PPLME_LIBPPLMECORE_PERSON_H_
