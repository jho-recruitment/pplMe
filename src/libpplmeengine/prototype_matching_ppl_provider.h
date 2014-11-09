/**
 *  @file
 *  @brief   Newfangled.
 *  @author  j.ho
 */
#ifndef PPLME_LIBPPLMEENGINE_PROTOTYPEMATCHINGPPLPROVIDER_H_
#define PPLME_LIBPPLMEENGINE_PROTOTYPEMATCHINGPPLPROVIDER_H_


#include "libpplmecore/matching_ppl_provider.h"
#include "libpplmeutils/pimpl.h"
#include "ppl_repository.h"


namespace pplme {
namespace engine {


/**
 *  Okay, so this is the lowdown.
 *
 *  The fundamental thing to understand is that this class divides the
 *  geography of Terra into discrete cells, where each cell corresponds to
 *  (1/resolution)^2 of a decimal degree.  This is the key way we search---
 *  we figure out what cell corresponds to that user, then we inspect the
 *  ppl in that cell, and neighbouring cells.  We also do some of this
 *  asychronously so that there is some modicum of scaling when queries are
 *  large (because max_distance is a silly value, or because the user is
 *  trolling or freezing their bits off near one of the Poles).
 *
 *  I found looking at a dartboard and imagining that the Bull was one of
 *  the Poles was quite enlightening when trying to see how the search works
 *  (or doesn't) near the singularity (no, not that one, sorry Kurzweil) points.
 *  YMMV.
 */
class PrototypeMatchingPplProvider :
      public PplRepository,
      public core::MatchingPplProvider {
 public:
  PrototypeMatchingPplProvider(
      int resolution,
      int max_distance,
      int max_age_difference,
      std::function<boost::gregorian::date()> date_provider);
  // Need noexcept to work-around gcc bug 53613.
  ~PrototypeMatchingPplProvider() noexcept (true);

  void Start();
  
  // It's unclear that having this copy constructible/assignable is desirable.
  PrototypeMatchingPplProvider(PrototypeMatchingPplProvider const&) = delete;
  PrototypeMatchingPplProvider& operator=(
      PrototypeMatchingPplProvider const&) = delete;

  void AddPerson(std::unique_ptr<core::Person> person) override;
  
  std::vector<core::Person>
  FindMatchingPpl(core::PplMatchingParameters const& parameters) const override;
  
 private:
  class Impl;
  utils::Pimpl<Impl> impl_;
};


}  // namespace engine
}  // namespace pplme


#endif  // PPLME_LIBPPLMEENGINE_PROTOTYPEMATCHINGPPLPROVIDER_H_
