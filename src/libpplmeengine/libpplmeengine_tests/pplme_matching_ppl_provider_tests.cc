/**
 *  @file
 *  @brief   Tests for pplme::engine::PplmeMatchingPplProvider.
 *  @author  j.ho
 */


#include <boost/numeric/conversion/cast.hpp>
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


/** @remarks  Using the default (of std::thread::hardware_concurrency()) turns
              out to be quite a bit slower than 1 on my dev box; however, we
              want to have some confidence that there are no races, so use 3
              as hopefully a reasonable compromise. */
int const kPerFindConcurrency = 3;


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
  int const kMaxPpl = 1;
  PplmeMatchingPplProvider ppl_provider(
      GetParam().resolution,
      GetParam().max_age_difference,
      kMaxPpl,
      kPerFindConcurrency,
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
  for (int const kGridResolution : { 1, 10 }) {
    // Create the MatchingPplProvider.
    PplmeMatchingPplProvider ppl_provider{
        kGridResolution,
        kMaxAgeDifference,
        GetParam().ppl.empty() ?
            1 : boost::numeric_cast<int>(GetParam().ppl.size()),
        kPerFindConcurrency,
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
  // "Four Corners of the Earth".
  PPLME_TESTLETTE({
      { "Alice", 90, 180 },
      { "Bob", -90, 180 },
      { "Charley", -90, -180 },
      { "Dave", 90, -180 }
    },
    0, 0,
    { "Alice", "Bob", "Charley", "Dave" }),

  // Different Corners.  &%D
  PPLME_TESTLETTE({
      { "Alice", 90, 0 },
      { "Bob", 0, 180 },
      { "Charley", 0, -180 },
      { "Dave", -90, -180 }
    },
    0, 0,
    { "Alice", "Bob", "Charley", "Dave" }),

  PPLME_TESTLETTE({}, 0, 0, {}),

  // Thank you <http://sqa.fyicenter.com/Online_Test_Tools/Random_Real_Number_Float_Value_Generator.php>.
  // And Trayport.
  // Forever <3.
  PPLME_TESTLETTE({
      { "Ayse", 24.2578060991, 69.1686814694 },
      { "Ben", -1.3534089324, 148.8467404101 },
      { "Chris", 59.342451696, -170.6773138562 },
      { "Digvijay", -80.0705690915, 23.9866400901 },
      { "Eddie", -58.5537641442, 120.3131025007 },
      { "FilB", 36.0427269275, 140.5816196094 },
      { "Gassman", -81.2790118769, 34.2015072397 },
      { "Hillary", -74.2717380562, 61.6072383717 },
      { "Iliyan", 85.2710097727, 19.7881726175 },
      { "Joe", -59.5607030436, -52.0936625973 }
    },
    0, 0,
    { "Ayse", "Ben", "Chris", "Digvijay", "Eddie",
      "FilB", "Gassman", "Hillary", "Iliyan", "Joe" }),

PPLME_TESTLETTES_END(find_all_matching_ppl_testlettes,
                     PplmeMatchingPplProviderTest_FindAllMatchingPpl)


/**
 *  @test  Test with Homer and The User at all the various permutations of
 *         ridiculously-quantized latitudes+longitudes.
 *  @remarks  We cheat somewhat here and autogenerate a boatload of Testlettes
 *            to save having to spell out the combos by hand (read as: Python).
 */
std::vector<FindAllMatchingPplTestlette> AllTheLatLongBigHitterCombos() {
  float const kLats[] = { -90, -45, 0, 45, 90 };
  float const kLongs[] = { -180, -120, -60, -30, 0, 30, 60, 120, 180 };

  std::vector<FindAllMatchingPplTestlette> testlettes;
  for (auto homerlat : kLats) {
    for (auto homerlong : kLongs) {
      for (auto userlat : kLats) {
        for (auto userlong : kLongs) {
          std::ostringstream testlette_desc;
          testlette_desc << "User (" << userlat << ", " << userlong
                         << ") -> Homer (" << homerlat << ", " << homerlong 
                         << ")";
          testlettes.push_back(FindAllMatchingPplTestlette{
              { { "Homer", homerlat, homerlong } },
              userlat, userlong,
              { "Homer" },
              testlette_desc.str()});
        }
      }
    }
  }

  return testlettes;
}
INSTANTIATE_TEST_CASE_P(AllTheLatLongBigHitterCombos,
                        PplmeMatchingPplProviderTest_FindAllMatchingPpl,
                        testing::ValuesIn(AllTheLatLongBigHitterCombos()));


/**
 *  @test  Test with Homer and The User at a multitude of random locations.
 */
std::vector<FindAllMatchingPplTestlette> RandomLatLongCombos() {
  int const kComboCount = 1024;

  std::default_random_engine random_engine;
  std::uniform_real_distribution<float> latgen{-90, 90};
  std::uniform_real_distribution<float> longgen{-180, 180};
  
  std::vector<FindAllMatchingPplTestlette> testlettes;
  for (int i = 0; i < kComboCount; ++i) {
    auto homerlat = latgen(random_engine);
    auto homerlong = longgen(random_engine);
    auto userlat = latgen(random_engine);
    auto userlong = longgen(random_engine);

    std::ostringstream testlette_desc;
    testlette_desc << "User (" << userlat << ", " << userlong
                   << ") -> Homer (" << homerlat << ", " << homerlong << ")";
    testlettes.push_back(FindAllMatchingPplTestlette{
        { { "Homer", homerlat, homerlong } },
          userlat, userlong,
          { "Homer" },
          testlette_desc.str()});
  }

  return testlettes;
}
INSTANTIATE_TEST_CASE_P(RandomLatLongCombos,
                        PplmeMatchingPplProviderTest_FindAllMatchingPpl,
                        testing::ValuesIn(RandomLatLongCombos()));


}  // namespace
