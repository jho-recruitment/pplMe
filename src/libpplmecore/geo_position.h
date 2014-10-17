/**
 *  @file
 *  @brief   Abstraction of a WGS 84-esque position on the planet Earth
 *           (latitude and longitude only; no elevation).
 *  @author  j.ho
 */
#ifndef PPLME_LIBPPLMECORE_GEOPOSITION_H_
#define PPLME_LIBPPLMECORE_GEOPOSITION_H_


#include "discriminated_value.h"


namespace pplme {
namespace core {


/**
 *  @remarks
 *  This type is optimized for size in the sense that positions are not
 *  represented particularly accurately.  This is good enough for pplMe
 *  because we are looking to discover people living nearby, not people
 *  that are within a 100m dash.
 */
class GeoPosition {
  struct DecimalLatitudeTag {};
  struct DecimalLongitudeTag {};
  
 public:
  /**
   *  Value type that represents a latitude or longitude in decimal degrees.
   *
   *  @remarks
   *  These might be better as more arithemtic types (e.g., employing sommat
   *  like Boost.Units) but since all arithmetic is currently effectively
   *  managed by this library, that doesn't matter too much, and using
   *  DiscriminatedValue means client code should be less prone to mixing up
   *  latitude and longitude.  Further, the use of a floating point type
   *  could be considered non-ideal (e.g., we could just store using an int32_t
   *  and have a multiplication factor of one million, or could use
   *  Boost.Rational); however, for the purposes of pplMe, any accuracy
   *  and/or precision concerns are largely irrelevant due to the fact that
   *  float is comfortably capable of representing kilometer distances.
   *  @{
   */
  typedef DiscriminatedValue<float, DecimalLatitudeTag> DecimalLatitude;
  typedef DiscriminatedValue<float, DecimalLongitudeTag> DecimalLongitude;
  /** @} */
  
  GeoPosition() : latitude_{}, longitude_{} {}
  GeoPosition(DecimalLatitude latitude, DecimalLongitude longitude)
      : latitude_{latitude}, longitude_{longitude} {}

  bool IsValid() const;

  DecimalLatitude latitude() const { return latitude_; }
  DecimalLongitude longitude() const { return longitude_; }

 private:
  DecimalLatitude latitude_;
  DecimalLongitude longitude_;
};


}  // namespace core
}  // namespace pplme


#endif  // PPLME_LIBPPLMECORE_GEOPOSITION_H_
