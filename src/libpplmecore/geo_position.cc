/**
 *  @file
 *  @brief   Implementation for pplme::core::GeoPosition.
 *  @author  j.ho
 */


#include "geo_position.h"


namespace pplme {
namespace core {


bool GeoPosition::IsValid() const {
  return latitude_.value() >= -90
      && latitude_.value() <= 90
      && longitude_.value() >= -180
      && longitude_.value() <= 180;
}


}  // namespace core
}  // namespace pplme
