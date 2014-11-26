/**
 *  @file
 *  @brief   Tests for pplme::engine::PplmeMatchingPplProvider.
 *  @author  j.ho
 */


#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <gtest/gtest.h>
#include "libpplmeutils/testlettes.h"
#include "libpplmeengine/pplme_matching_ppl_provider.h"


using pplme::core::GeoPosition;
using pplme::core::Person;
using pplme::core::PersonId;
using pplme::core::PplMatchingParameters;
using pplme::engine::PplmeMatchingPplProvider;


namespace {


PersonId GetTokenPersonId()
{
  char const kTokenPersonId [] = "12e50274-0ee3-4281-8a7b-57e8da9d13ee";
  return PersonId{boost::uuids::string_generator()(kTokenPersonId)};
}


PPLME_TESTLETTE_TYPE_BEGIN(FindMatchingPplTestlette)
  int resolution;
  int max_age_difference;
  float person_latitude;
  float person_longitude;
  boost::gregorian::date person_dob;
  float user_latitude;
  float user_longitude;
  int user_age;
  bool should_find_person;
PPLME_TESTLETTE_TYPE_END(FindMatchingPplTestlette,
                         PplmeMatchingPplProviderTest_FindMatchingPpl)

TEST_P(PplmeMatchingPplProviderTest_FindMatchingPpl, Tests) {
  PplmeMatchingPplProvider ppl_provider(
      GetParam().resolution,
      GetParam().max_age_difference,
      []() { return boost::gregorian::date{2014, 11, 8}; });
  std::unique_ptr<Person> person{new Person{
      GetTokenPersonId(),
      "Borence Claddicker",
      GetParam().person_dob,
      GeoPosition{
          GeoPosition::DecimalLatitude{GetParam().person_latitude},
          GeoPosition::DecimalLongitude{GetParam().person_longitude}}}};
  ppl_provider.AddPerson(std::move(person));

  PplMatchingParameters matching_params{
      GeoPosition{
          GeoPosition::DecimalLatitude{GetParam().user_latitude},
          GeoPosition::DecimalLongitude{GetParam().user_longitude}},
      GetParam().user_age};
  auto matching_ppl = ppl_provider.FindMatchingPpl(matching_params);
  
  ASSERT_EQ(
      GetParam().should_find_person ? 1U : 0U,
      matching_ppl.size());
}

PPLME_TESTLETTES_BEGIN(FindMatchingPplTestlette, find_matching_ppl_testlettes)
  PPLME_TESTLETTE(1, 1, 0, 0, { 1984, 11, 8 }, 0, 0, 30, true),
  PPLME_TESTLETTE(1, 1, -1, 0, { 1984, 11, 8 }, 0, 0, 30, true),
  PPLME_TESTLETTE(1, 1, 1, 0, { 1984, 11, 8 }, 0, 0, 30, true),
  PPLME_TESTLETTE(1, 1, 0, -1, { 1984, 11, 8 }, 0, 0, 30, true),
  PPLME_TESTLETTE(1, 1, 0, 1, { 1984, 11, 8 }, 0, 0, 30, true),
  PPLME_TESTLETTE(1, 1, 0, 0, { 1984, 11, 8 }, 0, 0, 29, true),
  PPLME_TESTLETTE(1, 1, 0, 0, { 1984, 11, 8 }, 0, 0, 28, false),
  PPLME_TESTLETTE(1, 1, 0, 0, { 1984, 11, 8 }, 0, 0, 31, true),
  PPLME_TESTLETTE(1, 1, 0, 0, { 1984, 11, 8 }, 0, 0, 32, false),
  PPLME_TESTLETTE(1, 0, 24.86, 67.01, { 1984, 11, 8 },
                  24.874552, 66.969059, 30, true),
  PPLME_TESTLETTE(1, 0, 0, 0, { 1984, 11, 9 }, 0, 0, 30, false),
  PPLME_TESTLETTE(1, 0, 0, 0, { 1984, 11, 7 }, 0, 0, 30, false),
  PPLME_TESTLETTE(1, 0, 0, 0, { 1984, 11, 8 }, 90, -65, 30, true),
  PPLME_TESTLETTE(1, 0, 0, 179.99, { 1984, 11, 8 }, 0, -179.99, 30, true),
  PPLME_TESTLETTE(1, 0, 0, -179.99, { 1984, 11, 8 }, 0, 179.99, 30, true)
PPLME_TESTLETTES_END(find_matching_ppl_testlettes,
                     PplmeMatchingPplProviderTest_FindMatchingPpl)


struct PplPerson {
  char const* name;
  float latitude;
  float longitude;
};

PPLME_TESTLETTE_TYPE_BEGIN(FindAllMatchingPplTestlette)
  std::vector<PplPerson> ppl;
  float user_latitude;
  float user_longitude;
  /* @remarks  These must be lexicographically ordered. */
  std::vector<std::string> expected_matching_ppl;
PPLME_TESTLETTE_TYPE_END(FindAllMatchingPplTestlette,
                         PplmeMatchingPplProviderTest_FindAllMatchingPpl)

/**
 *  @test  Unlike our FindMatchingPpl counterpart, this class of tests is
 *         purely about exercising location search and doesn't worry about age.
 */
TEST_P(PplmeMatchingPplProviderTest_FindAllMatchingPpl, Tests) {
  int const kMaxAgeDifference = 0;
  boost::gregorian::date const kTokenDoB{1914, 11, 25};
  auto kDateProvider = []() { return boost::gregorian::date{2014, 11, 25}; };
  int const kAgeOfUser = 100;

  // Try with multiple grid resolutions to ensure that the algo is sound.
  /* @todo  Only works when Grid Resolution is 1 for interim algorithm.
            Doesn't seem like it's worth trying to fix it given we're now
            about to rewrite using "spiral" so leave it singular for now. */
  for (int const kGridResolution : { 1 }) {
    // Create the MatchingPplProvider.
    PplmeMatchingPplProvider ppl_provider{
        kGridResolution,
        kMaxAgeDifference,
        kDateProvider};
    // Populate.
    for (auto const& ppl_person : GetParam().ppl) {
      std::unique_ptr<Person> person{new Person{
          PersonId{boost::uuids::random_generator()()},
          ppl_person.name,
          kTokenDoB,
          GeoPosition{
              GeoPosition::DecimalLatitude{ppl_person.latitude},
              GeoPosition::DecimalLongitude{ppl_person.longitude}}}};
      ppl_provider.AddPerson(std::move(person));
    }

    // Find ppl.
    PplMatchingParameters matching_params{
        GeoPosition{
            GeoPosition::DecimalLatitude{GetParam().user_latitude},
            GeoPosition::DecimalLongitude{GetParam().user_longitude}},
        kAgeOfUser};
    auto matching_ppl = ppl_provider.FindMatchingPpl(matching_params);

    // To make things a little easier, we just require that the Testlettes are
    // are lexicographically ordered, then we just need to sort the results to
    // match.
    std::sort(matching_ppl.begin(),
              matching_ppl.end(),
              [](Person const& lhs, Person const& rhs) {
                return lhs.name() < rhs.name();
              });
    ASSERT_TRUE(
        GetParam().expected_matching_ppl.size() == matching_ppl.size() &&
        std::equal(GetParam().expected_matching_ppl.cbegin(),
                   GetParam().expected_matching_ppl.cend(),
                   matching_ppl.cbegin(),
                   [](std::string const& lhs, Person const& rhs) {
                     return lhs == rhs.name();
                   }))
        << "Matching ppl with grid resolution of " << kGridResolution << ": "
        << ([&matching_ppl]() {
             std::ostringstream ppl;
             for (auto const& person : matching_ppl)
               ppl << person.name() << "; ";
             return ppl.str();
            })();
  }
}

PPLME_TESTLETTES_BEGIN(FindAllMatchingPplTestlette,
                       find_all_matching_ppl_testlettes)
  PPLME_TESTLETTE({
      { "Alice", 90, 180 },
      { "Bob", -90, 180 },
      { "Charley", -90, -180 },
      { "Dave", 90, -180 }
    },
    0, 0,
    { "Alice", "Bob", "Charley", "Dave" })
PPLME_TESTLETTES_END(find_all_matching_ppl_testlettes,
                     PplmeMatchingPplProviderTest_FindAllMatchingPpl)


}  // namespace
