/**
 *  @file
 *  @brief   Definition of functionality for converting between domain and
 *           proto UUID types.
 *  @author  j.ho
 */


#include <boost/uuid/uuid.hpp>
#include <glog/logging.h>
#include "convert_uuid.h"
#include "uuid.pb.h"


namespace pplme {
namespace proto {


void Convert(boost::uuids::uuid const& from, Uuid* to) {
  CHECK_NOTNULL(to);

  to->set_octets(&*from.begin(), from.size());
}


bool Convert(Uuid const& from, boost::uuids::uuid* to) {
  CHECK_NOTNULL(to);

  if (!from.has_octets()) {
    DLOG(ERROR) << "Uuid does not have octets";
    return false;
  }

  using boost::uuids::uuid;
  if (from.octets().size() != uuid::static_size()) {
    DLOG(ERROR) << "Uuid does not have " << uuid::static_size() << " octets";
    return false;
  }

  memcpy(to->data, from.octets().data(), sizeof(to->data));
  
  return true;
}


}  // namespace proto
}  // namespace pplme
