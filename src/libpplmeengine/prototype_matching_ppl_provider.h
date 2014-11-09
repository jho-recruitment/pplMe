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
