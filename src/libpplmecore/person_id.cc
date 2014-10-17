/**
 *  @file
 *  @brief   Implementation of PersonId-related functionality.
 *  @author  j.ho
 */


#include "person_id.h"
#include <boost/uuid/uuid_io.hpp>


namespace pplme {
namespace core {


bool IsValid(PersonId const& id)
{
  return !id.value().is_nil();
}


std::ostream& operator<<(std::ostream& stream, PersonId const& id)
{
  return stream << "PersonId(" << id.value() << ")";
}


}  // namespace core
}  // namespace pplme
