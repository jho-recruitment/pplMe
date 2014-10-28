/**
 *  @file
 *  @brief   Definition of functionality for converting between domain and
 *           proto GeoPosition types.
 *  @author  j.ho
 */


#include <glog/logging.h>
#include "libpplmecore/geo_position.h"
#include "convert_geo_position.h"
#include "geo_position.pb.h"


namespace pplme {
namespace proto {


void Convert(core::GeoPosition const& from, GeoPosition* to) {
  CHECK_NOTNULL(to);

  to->set_latitude(from.latitude().value());
  to->set_longitude(from.longitude().value());
}


bool Convert(GeoPosition const& from, core::GeoPosition* to) {
  CHECK_NOTNULL(to);

  if (!from.has_latitude()) {
    DLOG(ERROR) << "GeoPosition does not have a latitude";
    return false;
  }

  if (!from.has_longitude()) {
    DLOG(ERROR) << "GeoPosition does not have a longitude";
    return false;
  }
  
  *to = core::GeoPosition{
      core::GeoPosition::DecimalLatitude(from.latitude()),
      core::GeoPosition::DecimalLongitude(from.longitude())};

  return true;
}


}  // namespace proto
}  // namespace pplme
