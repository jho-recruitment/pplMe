/**
 *  @file
 *  @brief   A token implementation of MatchingPplProvider that is only capable
 *           of finding ppl within the same decimal degree-quantized
 *           latitude+longitude "coordinates".
 *  @author  j.ho
 */
#ifndef PPLME_LIBPPLMEENGINE_MY1STMATCHINGPPLPROVIDER_H_
#define PPLME_LIBPPLMEENGINE_MY1STMATCHINGPPLPROVIDER_H_


#include "libpplmecore/matching_ppl_provider.h"
#include "ppl_repository.h"


namespace pplme {
namespace engine {


/**
 *  @remarks
 *  This implementation is not great.  It fails to account for the edge cases
 *  such as 180 longitude == -180 longitude and also the fact that if a
 *  location is right near/on a boundary (e.g., has a non-fractional
 *  latitude and/or longitude), then there are going to be other ppl nearby
 *  in a neighbouring "square".  Also, of course, 1 degree longitude at the
 *  equator is actually a fair old distance.  &:D  Still, it serves as a
 *  useful token implementation for system/integration testing, and it'll
 *  do...for now....
 */
class My1stMatchingPplProvider :
      public PplRepository,
      public core::MatchingPplProvider {
 public:
  /**
   *  @param  max_age_difference is the maximum age difference between
   *          a user and another Person for them to be considered similar.
   *  @param  date_provider is responsible for providing the current date
   *          when figuring out matches (v.handy for unit tests (;&O).
   */
  explicit My1stMatchingPplProvider(
      int max_age_difference,
      std::function<boost::gregorian::date()> date_provider);
  // Need noexcept to work-around gcc bug 53613.
  ~My1stMatchingPplProvider() noexcept (true) {}

  // It's unclear that having this copy constructible/assignable is desirable.
  My1stMatchingPplProvider(My1stMatchingPplProvider const&) = delete;
  My1stMatchingPplProvider& operator=(My1stMatchingPplProvider const&) = delete;

  void AddPerson(std::unique_ptr<core::Person> person) override;
  
  std::vector<core::Person>
  FindMatchingPpl(core::PplMatchingParameters const& parameters) const override;
  
 private:
  std::function<boost::gregorian::date()> date_provider_;
  int max_age_difference_;
  /** This is a collection of collections of Person, where the outer collection
      effectively corresponds to a decimal degree-quantized latitude:longitude
      location, and the inner collection is the set of Person instances that
      live in that location. */
  std::vector<std::vector<std::unique_ptr<core::Person>>> ppl_;
};


}  // namespace engine
}  // namespace pplme


#endif  // PPLME_LIBPPLMEENGINE_MY1STMATCHINGPPLPROVIDER_H_
