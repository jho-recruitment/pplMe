/**
 *  @file
 *  @brief   Definition of functionality for converting between domain and
 *           proto Person types.
 *  @author  j.ho
 */


#include <glog/logging.h>
#include "libpplmecore/person.h"
#include "convert_date.h"
#include "convert_geo_position.h"
#include "convert_person.h"
#include "convert_uuid.h"
#include "person.pb.h"


namespace pplme {
namespace proto {


void Convert(core::Person const& from, Person* to) {
  CHECK_NOTNULL(to);

  Convert(from.id().value(), to->mutable_id());
  to->set_name(from.name());
  Convert(from.date_of_birth(), to->mutable_date_of_birth());
  Convert(from.location_of_home(), to->mutable_location_of_home());
}


bool Convert(Person const& from, core::Person* to) {
  CHECK_NOTNULL(to);

  if (!from.has_id()) {
    DLOG(ERROR) << "Person does not have id";
    return false;
  }
  if (!from.has_name()) {
    DLOG(ERROR) << "Person does not have name";
    return false;
  }
  if (!from.has_date_of_birth()) {
    DLOG(ERROR) << "Person does not have date of birth";
    return false;
  }
  if (!from.has_location_of_home()) {
    DLOG(ERROR) << "Person does not have location of home";
    return false;
  }

  boost::uuids::uuid id;
  if (!Convert(from.id(), &id))
    return false;
  boost::gregorian::date dob;
  if (!Convert(from.date_of_birth(), &dob))
    return false;
  core::GeoPosition home;
  if (!Convert(from.location_of_home(), &home))
    return false;

  *to = core::Person{core::PersonId{id}, from.name(), dob, home};
  
  return true;
}


}  // namespace proto
}  // namespace pplme
