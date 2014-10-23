/**
 *  @file
 *  @brief   Definition of the MatchingPplProvider interface, which is the
 *           fundamental interface in pplMe for finding people like someone.
 *  @author  j.ho
 */
#ifndef PPLME_LIBPPLMECORE_MATCHINGPPLPROVIDER_H_
#define PPLME_LIBPPLMECORE_MATCHINGPPLPROVIDER_H_


#include "person.h"
#include "ppl_matching_parameters.h"


namespace pplme {
namespace core {


/**
 *  @note
 *  Clients of this interface should feel free to ignore synchronization
 *  (i.e., any necessary synchronization should be assumed to be undertaken
 *  by the implementation).
 */
class MatchingPplProvider {
 public:
  virtual ~MatchingPplProvider() = default;

  /**
   *  Find a set of Person instances that match the specified @a parameters.
   */
  virtual std::vector<Person>
  FindMatchingPpl(PplMatchingParameters const& parameters) const = 0;
};


}  // namespace core
}  // namespace pplme


#endif  // PPLME_LIBPPLMECORE_MATCHINGPPLPROVIDER_H_
