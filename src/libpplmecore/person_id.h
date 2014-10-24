/**
 *  @file
 *  @brief   Definition of PersonId and related functionality.
 *  @author  j.ho
 */
#ifndef PPLME_LIBPPLMECORE_PERSONID_H_
#define PPLME_LIBPPLMECORE_PERSONID_H_


#include <boost/uuid/uuid.hpp>
#include "libpplmeutils/discriminated_value.h"


namespace pplme {
namespace core {


/** Tag type that functions as the DiscriminatingType for PersonId. */
struct PersonIdTag {};


/** Id type that uniquely identifies a Person. */
using PersonId = utils::DiscriminatedValue<boost::uuids::uuid, PersonIdTag>;

bool IsValid(PersonId const& id);

std::ostream& operator<<(std::ostream& stream, PersonId const& id);


}  // namespace core
}  // namespace pplme


#endif  // PPLME_LIBPPLMECORE_PERSONID_H_
