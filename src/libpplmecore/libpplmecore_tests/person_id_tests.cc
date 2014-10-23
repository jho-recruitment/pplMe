/**
 *  @file
 *  @brief   Tests for pplme::core::PersonId and friends.
 *  @author  j.ho
 */


#include <sstream>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <gtest/gtest.h>
#include "libpplmecore/person_id.h"


using pplme::core::PersonId;


namespace {

// <http://xkcd.com/221/>
char const kRandomUuid [] = "781efe11-fc00-4b02-8ad0-7995187143e0";

}  // namespace


TEST(PersonIdTest, IsValid)
{
  ASSERT_FALSE(IsValid(PersonId{boost::uuids::nil_generator()()}));
  ASSERT_TRUE(IsValid(PersonId{boost::uuids::string_generator()(kRandomUuid)}));
}


TEST(PersonIdTest, OutputStreamableness)
{
  boost::uuids::string_generator uuid_gen;
  std::ostringstream oss;
  oss << PersonId{uuid_gen(kRandomUuid)};
  
  ASSERT_TRUE(oss.str().find(kRandomUuid) != std::string::npos);
}
