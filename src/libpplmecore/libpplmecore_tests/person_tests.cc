/**
 *  @file
 *  @brief   Tests for pplme::core::Person.
 *  @author  j.ho
 */


#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <gtest/gtest.h>
#include "libpplmecore/person.h"


using pplme::core::GeoPosition;
using pplme::core::Person;
using pplme::core::PersonId;


TEST(PersonTest, DefaultCtor)
{
  Person person;

  ASSERT_FALSE(person.IsValid());
}


namespace {

PersonId GetTokenPersonId()
{
  char const kTokenPersonId [] = "25d27679-0b7e-44d4-a9f7-549fe39f3cd4";
  return PersonId{boost::uuids::string_generator()(kTokenPersonId)};
}

}  // namespace


TEST(PersonTest, NonDefaultCtor)
{
  GeoPosition const kTokenPos{GeoPosition::DecimalLatitude{45.6},
                              GeoPosition::DecimalLongitude{-90.1}};
  Person person{GetTokenPersonId(),
                "Baberaham Lincoln",
                boost::gregorian::date{2014, 10, 17},
                kTokenPos};

  ASSERT_EQ(GetTokenPersonId(), person.id());
  ASSERT_EQ("Baberaham Lincoln", person.name());
  ASSERT_EQ(boost::gregorian::date(2014, 10, 17), person.date_of_birth());
  ASSERT_EQ(kTokenPos.latitude(), person.location_of_home().latitude());
  ASSERT_EQ(kTokenPos.longitude(), person.location_of_home().longitude());
}


TEST(PersonTest, IsValid)
{
  Person person1{GetTokenPersonId(),
                 "", boost::gregorian::date(), GeoPosition()};
  Person person2{PersonId{boost::uuids::nil_generator()()},
                 "", boost::gregorian::date(), GeoPosition()};
  
  ASSERT_TRUE(person1.IsValid());
  ASSERT_FALSE(person2.IsValid());  
}
