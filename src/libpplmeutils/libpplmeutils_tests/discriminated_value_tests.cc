/**
 *  @file
 *  @brief   Tests for pplme::utils::DiscriminatedValue.
 *  @author  j.ho
 */


#include <sstream>
#include <gtest/gtest.h>
#include "libpplmeutils/discriminated_value.h"


namespace testsystem {

struct SystemIdTag {};

using SystemId = pplme::utils::DiscriminatedValue<uint32_t, SystemIdTag>;

bool IsValid(SystemId system_id)
{
  return system_id.value() != 0;
}

}  // namespace testsystem


TEST(DiscriminatedValueTest, DefaultCtor)
{
  testsystem::SystemId system_id{};

  ASSERT_EQ(uint32_t{}, system_id.value());
}


TEST(DiscriminatedValueTest, NonDefaultCtor)
{
  testsystem::SystemId system_id{20141016};

  ASSERT_EQ(uint32_t{20141016}, system_id.value());
}


TEST(DiscriminatedValueTest, set_value)
{
  testsystem::SystemId system_id{};

  system_id.set_value(61014102);

  ASSERT_EQ(uint32_t{61014102}, system_id.value());
}


TEST(DiscriminatedValueTest, ArgumentDependentLookup)
{
  testsystem::SystemId system_id{20141015};

  ASSERT_TRUE(IsValid(system_id));
}


TEST(DiscriminatedValueTest, OpEq)
{
  using testsystem::SystemId;
  ASSERT_TRUE(SystemId{20141017} == SystemId{20141017});
  ASSERT_FALSE(SystemId{20141017} == SystemId{71014102});
}


TEST(DiscriminatedValueTest, OpNe)
{
  using testsystem::SystemId;
  ASSERT_FALSE(SystemId{20141017} != SystemId{20141017});
  ASSERT_TRUE(SystemId{20141017} != SystemId{71014102});
}


TEST(DiscriminatedValueTest, OutputStreamableByDefault)
{
  std::ostringstream oss;
  oss.imbue(std::locale::classic());
  oss << testsystem::SystemId{20141017};
  
  ASSERT_TRUE(oss.str().find("20141017") != std::string::npos);
  // Current implementation should contain the discriminating type
  // (assumes vaguely sane std::type_info::name()).
  ASSERT_TRUE(oss.str().find("SystemIdTag") != std::string::npos);
}


namespace testsystemII {

struct SystemIdTag {};

using SystemId = pplme::utils::DiscriminatedValue<int32_t, SystemIdTag>;

std::ostream& operator<<(std::ostream& stream, SystemId)
{
  return stream << "Aardvark";
}

}  // namespace testsystemII

TEST(DiscriminatedValueTest, OutputStreamablenessMayBeOverridden)
{
  std::ostringstream oss;
  oss.imbue(std::locale::classic());
  oss << testsystemII::SystemId{20141017};
  
  ASSERT_EQ("Aardvark", oss.str());
}
