/**
 *  @file
 *  @brief   Implementation for pplme::engine::My1stMatchingPplProvider
 *  @author  j.ho
 */


#include "my_1st_matching_ppl_provider.h"


using pplme::core::GeoPosition;
using pplme::core::Person;


namespace {

// Longitude (-ve -> +ve) * Latitude (-ve -> +ve).
size_t const PplGridSize = 180 * 2 * 90 * 2;

/** Simple function that helps us resolve GeoPosition to a location within
    the ppl vector and avoids const_cast<> into the bargain. */
template <typename PplGridConster>
auto GetGridPos(PplGridConster& ppl, GeoPosition pos) -> decltype(ppl[0])&
{
  int latitude_index = pos.latitude().value() + 90;
  int longitude_index = pos.longitude().value() + 180;
  int grid_index = latitude_index * 360 + longitude_index;
  return ppl[grid_index];
}

}  // namespace


namespace pplme {
namespace engine {


My1stMatchingPplProvider::My1stMatchingPplProvider(
    int max_age_difference,
    std::function<boost::gregorian::date()> date_provider) :
    date_provider_{date_provider},
    max_age_difference_{max_age_difference},
    ppl_{PplGridSize}
{}


void My1stMatchingPplProvider::AddPerson(std::unique_ptr<Person> person)
{
  // We assume / don't-care if we've already seen a Person with the same id.
  GetGridPos(ppl_, person->location_of_home()).push_back(std::move(person));
}


std::vector<Person>
My1stMatchingPplProvider::FindMatchingPpl(
    core::PplMatchingParameters const& parameters) const
{
  std::vector<Person> ppl;

  auto const today = date_provider_();
  
  auto& grid_pos_ppl = GetGridPos(ppl_, parameters.location_of_user());
  for (auto const& person : grid_pos_ppl)
  {
    auto const earliest = today - boost::gregorian::years(
        parameters.age_of_user() + max_age_difference_);
    auto const latest = today - boost::gregorian::years(
        parameters.age_of_user() - max_age_difference_);
    // If they're in the same "grid position", then the Person is assumed to
    // be nearby, so the only other thing to consider is their age.
    if (   person->date_of_birth() >= earliest
        && person->date_of_birth() <= latest)
    {
      ppl.push_back(*person);
    }
  }

  return ppl;
}


}  // namespace engine
}  // namespace pplme
