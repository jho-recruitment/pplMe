/**
 *  @file
 *  @brief   Tests for pplme::engine::My1stMatchingPplProvider.
 *  @author  j.ho
 */


#include <boost/uuid/string_generator.hpp>
#include <gtest/gtest.h>
#include "libpplmeengine/my_1st_matching_ppl_provider.h"


using pplme::core::GeoPosition;
using pplme::core::Person;
using pplme::core::PersonId;
using pplme::core::PplMatchingParameters;
using pplme::engine::My1stMatchingPplProvider;


namespace {

PersonId GetTokenPersonId()
{
  char const kTokenPersonId [] = "7660eb9c-8962-40db-8a6f-65da2e3fe0b7";
  return PersonId{boost::uuids::string_generator()(kTokenPersonId)};
}

}  // namespace


namespace {

struct FindMatchingPplTestlette {
  int max_age_difference;
  float person_latitude;
  float person_longitude;
  boost::gregorian::date person_dob;
  float user_latitude;
  float user_longitude;
  int user_age;
  bool should_find_person;
}
  const find_matching_ppl_testlettes[] = {
    { 1, 0, 0, { 1984, 10, 23 }, 0, 0, 30, true },
    { 1, -1, 0, { 1984, 10, 23 }, 0, 0, 30, false },
    { 1, 1, 0, { 1984, 10, 23 }, 0, 0, 30, false },
    { 1, 0, -1, { 1984, 10, 23 }, 0, 0, 30, false },
    { 1, 0, 1, { 1984, 10, 23 }, 0, 0, 30, false },
    { 1, 0, 0, { 1984, 10, 23 }, 0, 0, 29, true },
    { 1, 0, 0, { 1984, 10, 23 }, 0, 0, 28, false },
    { 1, 0, 0, { 1984, 10, 23 }, 0, 0, 31, true },
    { 1, 0, 0, { 1984, 10, 23 }, 0, 0, 32, false },
    // Happy Birthday, Henry!  &:D
    { 0, 50.464503, -3.525291, { 2005, 10, 23 }, 50.464503, -3.525291, 9, true }
};

}  // namespace

class My1stMatchingPplProviderFindMatchingPpl :
      public testing::TestWithParam<FindMatchingPplTestlette> {};

TEST_P(My1stMatchingPplProviderFindMatchingPpl, FindMatchingPpl)
{
  My1stMatchingPplProvider ppl_provider(
      GetParam().max_age_difference,
      []() { return boost::gregorian::date{2014, 10, 23}; });
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

INSTANTIATE_TEST_CASE_P(Default,
                        My1stMatchingPplProviderFindMatchingPpl,
                        testing::ValuesIn(find_matching_ppl_testlettes));
