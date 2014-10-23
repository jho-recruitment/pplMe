/**
 *  @file
 *  @brief   Simple encapsulation of the parameters of a ppl matching operation.
 *  @author  j.ho
 */
#ifndef PPLME_LIBPPLMECORE_PPLMATCHINGPARAMETERS_H_
#define PPLME_LIBPPLMECORE_PPLMATCHINGPARAMETERS_H_


#include "geo_position.h"


namespace pplme {
namespace core {


/*
 *  See comment at top of file for a complete description.
 */
class PplMatchingParameters {
 public:
  PplMatchingParameters(GeoPosition location_of_user, int age_of_user)
      : location_of_user_{location_of_user}, age_of_user_{age_of_user} {}

  GeoPosition location_of_user() const { return location_of_user_; }
  int age_of_user() const { return age_of_user_; }

 private:
  GeoPosition location_of_user_;
  int age_of_user_;
};


}  // namespace core
}  // namespace pplme


#endif  // PPLME_LIBPPLMECORE_PPLMATCHINGPARAMETERS_H_
