/**
 *  @file
 *  @brief   Definition of PersonId and related functionality.
 *  @author  j.ho
 */
#ifndef PPLME_LIBPPLMECORE_PERSONID_H_
#define PPLME_LIBPPLMECORE_PERSONID_H_


#include <boost/uuid/uuid.hpp>
#include "discriminated_value.h"


namespace pplme {
namespace core {

namespace detail {
struct PersonIdTag {};
}  // namespace detail

using PersonId = DiscriminatedValue<boost::uuids::uuid, detail::PersonIdTag>;

bool IsValid(PersonId const& id);

std::ostream& operator<<(std::ostream& stream, PersonId const& id);


}  // namespace core
}  // namespace pplme


#endif  // PPLME_LIBPPLMECORE_PERSONID_H_
