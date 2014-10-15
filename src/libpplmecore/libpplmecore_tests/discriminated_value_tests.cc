/**
 *  @file
 *  @brief   Tests for pplme::core::DiscriminatedValue.
 *  @author  j.ho
 */


#include <ostream>
#include <gtest/gtest.h>
#include "libpplmecore/discriminated_value.h"


namespace testsystem {

struct SystemIdTag {};

using SystemId = pplme::core::DiscriminatedValue<uint32_t, SystemIdTag>;

bool IsValid(SystemId system_id)
{
  return system_id.value() != 0;
}

}  // namespace testsystem


TEST(DiscriminatedValue, DefaultCtor)
{
  testsystem::SystemId system_id{};

  ASSERT_EQ(uint32_t{}, system_id.value());
}


TEST(DiscriminatedValue, NonDefaultCtor)
{
  testsystem::SystemId system_id{20141016};

  ASSERT_EQ(uint32_t{20141016}, system_id.value());
}


TEST(DiscriminatedValue, set_value)
{
  testsystem::SystemId system_id{};

  system_id.set_value(61014102);

  ASSERT_EQ(uint32_t{61014102}, system_id.value());
}


TEST(DiscriminatedValue, ArgumentDependentLookup)
{
  testsystem::SystemId system_id{20141015};

  ASSERT_TRUE(IsValid(system_id));
}
