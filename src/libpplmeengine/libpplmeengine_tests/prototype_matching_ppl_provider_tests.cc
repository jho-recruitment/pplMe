/**
 *  @file
 *  @brief   Tests for pplme::engine::PrototypeMatchingPplProvider.
 *  @author  j.ho
 */


#include <boost/uuid/string_generator.hpp>
#include <gtest/gtest.h>
#include "libpplmeutils/testlettes.h"
#include "libpplmeengine/prototype_matching_ppl_provider.h"


using pplme::core::GeoPosition;
using pplme::core::Person;
using pplme::core::PersonId;
using pplme::core::PplMatchingParameters;
using pplme::engine::PrototypeMatchingPplProvider;


namespace {


PersonId GetTokenPersonId()
{
  char const kTokenPersonId [] = "7660eb9c-8962-40db-8a6f-65da2e3fe0b7";
  return PersonId{boost::uuids::string_generator()(kTokenPersonId)};
}


PPLME_TESTLETTE_TYPE_BEGIN(FindMatchingPplTestlette)
  int resolution;
  int max_distance;
  int max_age_difference;
  float person_latitude;
  float person_longitude;
  boost::gregorian::date person_dob;
  float user_latitude;
  float user_longitude;
  int user_age;
  bool should_find_person;
PPLME_TESTLETTE_TYPE_END(FindMatchingPplTestlette,
                         PrototypeMatchingPplProviderTest_FindMatchingPpl)

TEST_P(PrototypeMatchingPplProviderTest_FindMatchingPpl, Tests) {
  PrototypeMatchingPplProvider ppl_provider(
      GetParam().resolution,
      GetParam().max_distance,
      GetParam().max_age_difference,
      []() { return boost::gregorian::date{2014, 11, 8}; });
  std::unique_ptr<Person> person{new Person{
      GetTokenPersonId(),
      "Clarence Boddicker",
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
  PPLME_TESTLETTE(1, 10, 1, 0, 0, { 1984, 11, 8 }, 0, 0, 30, true ),
  PPLME_TESTLETTE(1, 10, 1, -1, 0, { 1984, 11, 8 }, 0, 0, 30, false ),
  PPLME_TESTLETTE(1, 10, 1, 1, 0, { 1984, 11, 8 }, 0, 0, 30, false ),
  PPLME_TESTLETTE(1, 10, 1, 0, -1, { 1984, 11, 8 }, 0, 0, 30, false ),
  PPLME_TESTLETTE(1, 10, 1, 0, 1, { 1984, 11, 8 }, 0, 0, 30, false ),
  PPLME_TESTLETTE(1, 10, 1, 0, 0, { 1984, 11, 8 }, 0, 0, 29, true ),
  PPLME_TESTLETTE(1, 10, 1, 0, 0, { 1984, 11, 8 }, 0, 0, 28, false ),
  PPLME_TESTLETTE(1, 10, 1, 0, 0, { 1984, 11, 8 }, 0, 0, 31, true ),
  PPLME_TESTLETTE(1, 10, 1, 0, 0, { 1984, 11, 8 }, 0, 0, 32, false ),
  PPLME_TESTLETTE(1, 10, 0, 24.86, 67.01, { 1984, 11, 8 },
                  24.874552, 66.969059, 30, true),
  PPLME_TESTLETTE(1, 4, 0, 24.86, 67.01, { 1984, 11, 8 },
                  24.874552, 66.969059, 30, false)
PPLME_TESTLETTES_END(find_matching_ppl_testlettes,
                     PrototypeMatchingPplProviderTest_FindMatchingPpl)


}  // namespace
