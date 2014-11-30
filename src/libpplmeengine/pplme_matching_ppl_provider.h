/**
 *  @file
 *  @brief   The new^H^H^HMatchingPplProvider of choice for pplMe.
 *           (This one even implements the Exercise Requirements.  Such wow!)
 *  @author  j.ho
 */
#ifndef PPLME_LIBPPLMEENGINE_PPLMEMATCHINGPPLPROVIDER_H_
#define PPLME_LIBPPLMEENGINE_PPLMEMATCHINGPPLPROVIDER_H_


#include <boost/optional.hpp>
#include "libpplmecore/matching_ppl_provider.h"
#include "libpplmeutils/pimpl.h"
#include "ppl_repository.h"


namespace pplme {
namespace engine {


/**
 *  @todoco ...
 */
class PplmeMatchingPplProvider :
      public PplRepository,
      public core::MatchingPplProvider {
 public:
  PplmeMatchingPplProvider(
      int resolution,
      int max_age_difference,
      int max_ppl,
      boost::optional<int> per_find_concurrency,
      std::function<boost::gregorian::date()> date_provider);
  // Need noexcept to work-around gcc bug 53613.
  ~PplmeMatchingPplProvider() noexcept (true);

  void Start();
  
  // It's unclear that having this copy constructible/assignable is desirable.
  PplmeMatchingPplProvider(PplmeMatchingPplProvider const&) = delete;
  PplmeMatchingPplProvider& operator=(PplmeMatchingPplProvider const&) = delete;

  void AddPerson(std::unique_ptr<core::Person> person) override;
  
  std::vector<core::Person>
  FindMatchingPpl(core::PplMatchingParameters const& parameters) const override;
  
 private:
  class Impl;
  utils::Pimpl<Impl> impl_;
};


}  // namespace engine
}  // namespace pplme


#endif  // PPLME_LIBPPLMEENGINE_PPLMEMATCHINGPPLPROVIDER_H_
